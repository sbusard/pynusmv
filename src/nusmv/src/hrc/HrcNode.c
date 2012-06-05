/**CFile***********************************************************************

  FileName    [HrcNode.c]

  PackageName [hrc]

  Synopsis    [Implementation of class 'HrcNode']

  Description []

  SeeAlso     [HrcNode.h]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK-irst.

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

  Revision    [$Id: HrcNode.c,v 1.1.2.18 2010-02-05 14:12:33 nusmv Exp $]

******************************************************************************/

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "utils/utils.h"
#include "node/node.h"
#include "utils/Slist.h"
#include "compile/symb_table/SymbTable.h"
#include "compile/compile.h"
#include "parser/symbols.h"
#include "HrcNode.h"
#include "hrcInt.h"

#include "utils/assoc.h"

static char rcsid[] UTIL_UNUSED = "$Id: HrcNode.c,v 1.1.2.18 2010-02-05 14:12:33 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [HrcNode class definition]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct HrcNode_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  SymbTable_ptr st;           /* The symbol table */
  int lineno;                 /* line number of the module */
  node_ptr name;              /* The name of the module */
  node_ptr instance_name;     /* The instance name */
  HrcNode_ptr parent;         /* The pointer to the parent node */
  node_ptr formal_parameters; /* formal parameters */
  node_ptr actual_parameters; /* actual parameters */
  node_ptr state_variables;   /* state variables */
  node_ptr input_variables;   /* input variables */
  node_ptr frozen_variables;  /* frozen variables */
  node_ptr defines;           /* DEFINE x := */
  node_ptr array_defines;         /* ARRAY DEFINE x := */
  node_ptr init_expr;         /* init expression INIT */
  node_ptr init_assign;       /* init assignements init(x) :=.. */
  node_ptr invar_expr;        /* init expression INVAR */
  node_ptr invar_assign;      /* init assignements x :=.. */
  node_ptr next_expr;         /* init expression TRANS */
  node_ptr next_assign;       /* init assignements next(x) :=.. */
  node_ptr justice;           /* JUSTICE/FAIRNESS */
  node_ptr compassion;        /* COMPASSION */
  node_ptr constants;         /* CONSTANTS */
  node_ptr invar_props;       /* INVARSPEC */
  node_ptr ctl_props;         /* CTLSPEC */
  node_ptr ltl_props;         /* LTLSPEC */
  node_ptr psl_props;         /* PSLSPEC */
  node_ptr compute_props;     /* COMPUTE */
  Slist_ptr childs;           /* List of sub-childs */
  hash_ptr assigns_table;     /* Assignments hash (left part -> right part) */
  void * undef;               /* For programmers use. Here additional
                                 information can be attached for
                                 several use without having to modify
                                 the structure */
} HrcNode;



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

HrcNode_ptr mainHrcNode;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Free a list and set its pointer to nil]

  Description  []

  SideEffects  [List is freed.]

  SeeAlso      []

******************************************************************************/
#define FREELIST_AND_SET_TO_NIL(list)           \
  free_list(list);                              \
  list = Nil;

/**Macro***********************************************************************

  Synopsis     [Free a list and all its elements, settin the list
  pointer to nil.]

  Description  [Free a list and all its elements, settin the list
  pointer to nil.]

  SideEffects  [Elements in list are freed, list is freed.]

  SeeAlso      []

******************************************************************************/
#define FREE_CONS_LIST_AND_SET_TO_NIL(list)             \
  hrc_node_free_cons_elements_in_list_and_list(list);   \
  list = Nil;


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void hrc_node_init ARGS((HrcNode_ptr self));
static void hrc_node_deinit ARGS((HrcNode_ptr self));
static node_ptr hrc_node_copy_cons_list ARGS((node_ptr cons_list));
static void hrc_node_free_cons_elements_in_list_and_list ARGS((node_ptr list));

static void hrc_node_free_list_and_clear_assign_map ARGS((HrcNode_ptr self,
                                                          int assign_type));

static assoc_retval hrc_node_free_cons_map_fun ARGS((char *key,
                                                     char *data,
                                                     char * arg));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The HrcNode class constructor]

  Description        [The HrcNode class constructor]

  SideEffects        []

  SeeAlso            [HrcNode_destroy]

******************************************************************************/
HrcNode_ptr HrcNode_create()
{
  HrcNode_ptr self = ALLOC(HrcNode, 1);
  HRC_NODE_CHECK_INSTANCE(self);

  hrc_node_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The HrcNode class destructor]

  Description        [The HrcNode class destructor]

  SideEffects        [The node is freed]

  SeeAlso            [HrcNode_create]

******************************************************************************/
void HrcNode_destroy(HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  hrc_node_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [The HrcNode class destructor. It recurses on the childs.]

  Description        [The HrcNode class destructor. It recurses on the childs.]

  SideEffects        [The whole hierarchy tree is freed]

  SeeAlso            [HrcNode_create, HrcNode_destroy]

******************************************************************************/
void HrcNode_destroy_recur(HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  Slist_ptr child = HrcNode_get_child_hrc_nodes(self);
  Siter iter;

  child = HrcNode_get_child_hrc_nodes(self);

  for(iter = Slist_first(child);
      false == Siter_is_end(iter);
      iter = Siter_next(iter)) {
    HrcNode_ptr c;

    c = (HrcNode_ptr)Siter_element(iter);

    HrcNode_destroy_recur(c);
  }
  HrcNode_destroy(self);
}


/**Function********************************************************************

  Synopsis           [Resets all fields of the given node]

  Description        [Resets all fields of the given node.
                      This is needed for safely recycle a node instance.
                      For example, if a parsing error occurs.
                      Children are destroyed.
                      ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcNode_cleanup(HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  /* members initialization */
  self->st = SYMB_TABLE(NULL);
  self->lineno = 0;
  self->name = Nil;
  self->instance_name = Nil;
  self->parent = HRC_NODE(NULL);
  self->formal_parameters = Nil;
  self->actual_parameters = Nil;
  self->state_variables = Nil;
  self->input_variables = Nil;
  self->frozen_variables = Nil;
  self->defines = Nil;
  self->array_defines = Nil;
  self->init_expr = Nil;
  self->init_assign = Nil;
  self->invar_expr = Nil;
  self->invar_assign = Nil;
  self->next_expr = Nil;
  self->next_assign = Nil;
  self->justice = Nil;
  self->compassion = Nil;
  self->constants = Nil;
  self->invar_props = Nil;
  self->ctl_props = Nil;
  self->ltl_props = Nil;
  self->psl_props = Nil;
  self->compute_props = Nil;
  self->undef = (void*)NULL;

  clear_assoc_and_free_entries(self->assigns_table,
                               hrc_node_free_cons_map_fun);

  {
    Slist_ptr child = HrcNode_get_child_hrc_nodes(self);
    Siter iter;

    child = HrcNode_get_child_hrc_nodes(self);

    for(iter = Slist_first(child);
        false == Siter_is_end(iter);
        iter = Siter_next(iter)) {
      HrcNode_ptr c;

      c = (HrcNode_ptr)Siter_element(iter);

      HrcNode_destroy_recur(c);
    }
  }
}


/**Function********************************************************************

  Synopsis           [Sets the symbol table inside the node.]

  Description        [Sets the symbol table inside the node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_symbol_table(HrcNode_ptr self, SymbTable_ptr st)
{
  HRC_NODE_CHECK_INSTANCE(self);
  self->st = st;
}

/**Function********************************************************************

  Synopsis           [Gets the symbol table.]

  Description        [Gets the symbol table.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
SymbTable_ptr HrcNode_get_symbol_table(HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(SYMB_TABLE(NULL) != self->st);
  return self->st;
}

/**Function********************************************************************

  Synopsis           [Sets the MOD_TYPE lineno of the node.]

  Description        [Sets the MOD_TYPE lineno of the node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_lineno(HrcNode_ptr self, int lineno)
{
  HRC_NODE_CHECK_INSTANCE(self);
  self->lineno = lineno;
}

/**Function********************************************************************

  Synopsis           [Gets the MOD_TYPE lineno of the node.]

  Description        [Gets the MOD_TYPE lineno of the node.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
int HrcNode_get_lineno(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->lineno;
}

/**Function********************************************************************

  Synopsis           [Sets the MOD_TYPE name of the node.]

  Description        [Sets the MOD_TYPE name of the node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_name(HrcNode_ptr self, node_ptr name)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->name);
  self->name = name;
}

/**Function********************************************************************

  Synopsis           [Gets the MOD_TYPE name of the node.]

  Description        [Gets the MOD_TYPE name of the node. WARNING: the returned 
  name is 'normalized' and can be used as hash value.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_name(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return find_atom(self->name);
}

/**Function********************************************************************

  Synopsis           [Gets the MOD_TYPE name of the node.]

  Description [Gets the MOD_TYPE name of the node. WARNING: the
  returned name is the name passed to SetName, and it is not
  'normalized' like in GetName. This can be used to obtain the node
  as produced by the parser.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_crude_name(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->name;
}

/**Function********************************************************************

  Synopsis           [Sets the instance name of the node.]

  Description        [Sets the instance name of the node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_instance_name(HrcNode_ptr self, node_ptr name)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->instance_name);
  self->instance_name = name;
}

/**Function********************************************************************

  Synopsis           [Gets the instance name of the node.]

  Description        [Gets the instance name of the node.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_instance_name(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->instance_name;
}

/**Function********************************************************************

  Synopsis           [Gets the flattened instance name of the node.]

  Description        [Gets the flattened instance name of the node.

  The hierarchy is visited upward from self until main node is
  found. The flattened and normalized instance node is built and
  returned.

  The flattened instance name is the name obtained considering all the
  anchestors instance of the current node.

  The result of this operation could also be memoized to improve
  performances (this would avoid to recompute the same path twice).]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr HrcNode_get_flattened_instance_name(const HrcNode_ptr self)
{
  node_ptr flattened_name;
  HrcNode_ptr instance_iter;
  Slist_ptr variables_chain;

  HRC_NODE_CHECK_INSTANCE(self);

  variables_chain = Slist_create();

  /* The hierarchy is visited upward until the root node.
     A stack of variables is created. At the top of the stack there should
     be the main module.
   */
  instance_iter = self;
  while (HRC_NODE(Nil) != instance_iter) {
    node_ptr instance_name;

    instance_name = HrcNode_get_instance_name(instance_iter);
    Slist_push(variables_chain, instance_name);

    instance_iter = HrcNode_get_parent(instance_iter);
  }

  /* From the variables stack creates the DOT tree for the flattened name
     of the instance.

     [SM] More efficient if popping out all variables!
  */
  flattened_name = Nil;

  while (! Slist_is_empty(variables_chain)) {
    node_ptr current_var;

    current_var = Slist_pop(variables_chain);
    if (NODE_PTR(Nil) != current_var) {
      /*
        The instance is not the main module.

        Here we use CompileFlatten_concat_contexts for two reasons:
          - find_node: is not able to manage instante names with
        intermediate dots.
          - CompileFlatten_resolve_name, ecc...: they do not handle
        instance names but only variables.
      */
      flattened_name = CompileFlatten_concat_contexts(flattened_name,
                                               find_atom(current_var));
    }
  }

  Slist_destroy(variables_chain);

  return flattened_name;
}

/**Function********************************************************************

  Synopsis           [Sets the parent node of the node.]

  Description        [Sets the parent node of the node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_parent(HrcNode_ptr self, const HrcNode_ptr father)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(HRC_NODE(NULL) == self->parent);
  self->parent = father;

  return;
}

/**Function********************************************************************

  Synopsis           [Get the parent node of the node.]

  Description        [Get the parent node of the node. HRC_NODE(NULL)
  is returned if no father available.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
HrcNode_ptr HrcNode_get_parent(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->parent;
}

/**Function********************************************************************

  Synopsis           [Sets the formal parameters of the current node.]

  Description        [Sets the formal parameters of the current
  node. They are list of pairs (name . type), where type specifies the
  type of the parameter if know, otherwise it is Nil.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_formal_parameters(HrcNode_ptr self, node_ptr par)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->formal_parameters);

  self->formal_parameters = par;
}

/**Function********************************************************************

  Synopsis           [Replaces the formal parameters of the current node.]

  Description        [Relaces the formal parameters of the current node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_formal_parameters(HrcNode_ptr self, node_ptr par)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->formal_parameters);

  HrcNode_set_formal_parameters(self, par);
}

/**Function********************************************************************

  Synopsis           [Gets the formal parameters of the current node.]

  Description        [Gets the formal parameters of the current
  node. They result os a list of pairs (name . type), where type
  specifies the type of the parameter if know, otherwise it is Nil.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_formal_parameters(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->formal_parameters;
}

/**Function********************************************************************

  Synopsis           [Adds a formal parameter to the the current node.]

  Description        [Adds a formal parameter to the current node. It
  should be a pair (name . type), where type specifies the type of the
  parameter if known, otherwise it is Nil.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_formal_parameter(HrcNode_ptr self, node_ptr par)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->formal_parameters = cons(par, self->formal_parameters);
}

/**Function********************************************************************

  Synopsis           [Sets the actual parameters of the current node.]

  Description        [Sets the actual parameters of the current
  node. They are list of pairs (name . expr), where expr specifies the
  expression the current current formal parameter node has been
  instatiated to.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_actual_parameters(HrcNode_ptr self, node_ptr par)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->actual_parameters);

  self->actual_parameters = par;
}

/**Function********************************************************************

  Synopsis           [Replaces the actual parameters of the current node.]

  Description        [Replaces the actual parameters of the current node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_actual_parameters(HrcNode_ptr self, node_ptr par)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->actual_parameters);

  HrcNode_set_actual_parameters(self, par);
}



/**Function********************************************************************

  Synopsis           [Gets the actual parameters of the current node.]

  Description        [Gets the actual parameters of the current
  node. The result is a list of pairs (name . expr), where expr specifies the
  expression the current current formal parameter node has been
  instatiated to.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_actual_parameters(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->actual_parameters;
}


/**Function********************************************************************

  Synopsis           [Adds an actual parameter to the current node.]

  Description        [Adds an actual parameter to the current node. It
  should be a pair (name . expr), where expr specifies the expression the
  parameter has been instantiated to.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_actual_parameter(HrcNode_ptr self, node_ptr par)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->actual_parameters = cons(par, self->actual_parameters);
}

/**Function********************************************************************

  Synopsis           [Sets the local state variables of the current node.]

  Description        [Sets the local state variables of the current
  node. It is a list of pairs (name . type) where type is the type of
  the corresponding variable.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_state_variables(HrcNode_ptr self, node_ptr vars)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->state_variables);
  self->state_variables = vars;
}

/**Function********************************************************************

  Synopsis           [Replaces the local state variables of the current node.]

  Description        [Replaces the local state variables of the current node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_state_variables(HrcNode_ptr self, node_ptr vars)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->state_variables);

  HrcNode_set_state_variables(self,vars);
}

/**Function********************************************************************

  Synopsis           [Gets the local state variables of the current node.]

  Description        [Gets the local state variables of the current
  node. The result is a list of pairs (name . type) where type is the
  type of the corresponding variable.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_state_variables(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->state_variables;
}

/**Function********************************************************************

  Synopsis           [Adds a state variable to the current node.]

  Description        [Adds a state variable to the current node. The
  var should be a pairs (name . type) where type is the type of the
  corresponding variable.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_state_variable(HrcNode_ptr self, node_ptr var)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->state_variables = cons(var, self->state_variables);
}

/**Function********************************************************************

  Synopsis           [Sets the local input variables of the current node.]

  Description        [Sets the local input variables of the current
  node. It is a list of pairs (name . type) where type is the type of
  the corresponding variable.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_input_variables(HrcNode_ptr self, node_ptr vars)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->input_variables);
  self->input_variables = vars;
}

/**Function********************************************************************

  Synopsis           [Replacess the local input variables of the current node.]

  Description        [Replaces the local input variables of the current  node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_input_variables(HrcNode_ptr self, node_ptr vars)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->input_variables);

  HrcNode_set_input_variables(self, vars);
}

/**Function********************************************************************

  Synopsis           [Gets the local input variables of the current node.]

  Description        [Gets the local input variables of the current
  node. The result is a list of pairs (name . type) where type is the
  type of the corresponding variable.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_input_variables(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->input_variables;
}

/**Function********************************************************************

  Synopsis           [Adds a input variable to the current node.]

  Description        [Adds a input variable to the current node. The
  var should be a pairs (name . type) where type is the type of the
  corresponding variable.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_input_variable(HrcNode_ptr self, node_ptr var)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->input_variables = cons(var, self->input_variables);
}


/**Function********************************************************************

  Synopsis           [Sets the local frozen variables of the current node.]

  Description        [Sets the local frozen variables of the current
  node. It is a list of pairs (name . type) where type is the type of
  the corresponding variable.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_frozen_variables(HrcNode_ptr self, node_ptr vars)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->frozen_variables);
  self->frozen_variables = vars;
}


/**Function********************************************************************

  Synopsis           [Replaces the local frozen variables of the
  current node.]

  Description        [Replaces the local frozen variables of the
  current node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_frozen_variables(HrcNode_ptr self, node_ptr vars)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->frozen_variables);

  HrcNode_set_frozen_variables(self, vars);
}

/**Function********************************************************************

  Synopsis           [Gets the local frozen variables of the current node.]

  Description        [Gets the local frozen variables of the current
  node. The result is a list of pairs (name . type) where type is the
  type of the corresponding variable.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_frozen_variables(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->frozen_variables;
}

/**Function********************************************************************

  Synopsis           [Adds a frozen variable to the current node.]

  Description        [Adds a frozen variable to the current node. The
  var should be a pairs (name . type) where type is the type of the
  corresponding variable.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_frozen_variable(HrcNode_ptr self, node_ptr var)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->frozen_variables = cons(var, self->frozen_variables);
}

/**Function********************************************************************

  Synopsis           [Sets the local DEFINES of the current node.]

  Description        [Sets the local DEFINES for the current node. It
  is a list of pairs (name . expr) where expr is the body of the
  DEFINEd symbol.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_defines(HrcNode_ptr self, node_ptr defs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->defines);
  self->defines = defs;
}

/**Function********************************************************************

  Synopsis           [Replaces the local DEFINES of the current node.]

  Description        [Replaces the local DEFINES for the current node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_defines(HrcNode_ptr self, node_ptr defs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->defines);

  HrcNode_set_defines(self, defs);
}

/**Function********************************************************************

  Synopsis           [Gets the local DEFINES of the current node.]

  Description        [Gets the local DEFINES of the current node. The
  result is a list of pairs (name . expr) where expr is the body of
  the DEFINEd symbol.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_defines(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->defines;
}


/**Function********************************************************************

  Synopsis           [Adds a DEFINE to the current node.]

  Description        [Adds a define declaration to the current node. The
  define should be a pairs (name . expr) where expr is the body of the
  current DEFINE symbol.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_define(HrcNode_ptr self, node_ptr def)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->defines = cons(def, self->defines);
}

/**Function********************************************************************

  Synopsis           [Set the local  ARRAY DEFINES of the current node.]

  Description        [Sets the local ARRAY DEFINES for the current node. It
  is a list of pairs (name . expr) where expr is the body of the
  ARRAY DEFINEd symbol.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_array_defines(HrcNode_ptr self, node_ptr mdefs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->array_defines);
  self->array_defines = mdefs;
}


/**Function********************************************************************

  Synopsis           [Replaces the local ARRAY DEFINES of the current
  node.]

  Description        [Replaces the local ARRAY DEFINES for the current
  node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_array_defines(HrcNode_ptr self, node_ptr mdefs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->array_defines);

  HrcNode_set_array_defines(self, mdefs);
}

/**Function********************************************************************

  Synopsis           [Gets the local ARRAY DEFINES of the current node.]

  Description        [Gets the local ARRAY DEFINES of the current node. The
  result is a list of pairs (name . expr) where expr is the body of
  the ARRAY DEFINEd symbol.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_array_defines(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->array_defines;
}

/**Function********************************************************************

  Synopsis           [Adds an ARRAY DEFINE to the current node.]

  Description        [Adds a ARRAY DEFINE declaration to the current
  node. The array define should be a pairs (name . expr) where expr is the
  body of the  current DEFINE symbol.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_array_define(HrcNode_ptr self, node_ptr mdef)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->array_defines = cons(mdef, self->array_defines);
}


/**Function********************************************************************

  Synopsis           [Sets the INIT expressions for the current node.]

  Description        [Sets the INIT expressions for the current
  node. It is a list of implicitly conjoined expressions.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_init_exprs(HrcNode_ptr self, node_ptr exprs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->init_expr);
  self->init_expr = exprs;
}

/**Function********************************************************************

  Synopsis           [Replaces the INIT expressions for the current
  node.]

  Description        [Replaces the INIT expressions for the current
  node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_init_exprs(HrcNode_ptr self, node_ptr exprs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->init_expr);

  HrcNode_set_init_exprs(self, exprs);
}

/**Function********************************************************************

  Synopsis           [Sets the INIT expressions for the current node.]

  Description        [Sets the INIT expressions for the current
  node. It is a possibly empty list of implicitly conjoined expressions.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_init_exprs(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->init_expr;
}

/**Function********************************************************************

  Synopsis           [Adds an INIT expression to the current node.]

  Description        [Adds an INIT expression to the current node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_init_expr(HrcNode_ptr self, node_ptr expr)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->init_expr = cons(expr, self->init_expr);
}

/**Function********************************************************************

  Synopsis           [Sets the init(*) := expressions for the current node.]

  Description        [Sets the init(*) := expressions for the current
  node. It is a list of implicitly conjoined assignements.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_init_assign_exprs(HrcNode_ptr self, node_ptr assigns)
{
  HRC_NODE_CHECK_INSTANCE(self);

  node_ptr tmp_assigns;

  nusmv_assert(Nil == self->init_assign);

  /* Avoids to rewrite assigns */
  tmp_assigns = assigns;
  while (Nil != tmp_assigns) {
    node_ptr ass = find_assoc(self->assigns_table, car(car(tmp_assigns)));

    if (Nil != ass) {
      /* We can have both SMALLINIT and NEXT, but not INVAR and
         INIT/NEXT nor 2 INIT assigns */
      nusmv_assert(NODE_FROM_INT(NEXT) == car(ass));
      setcdr(ass, NODE_FROM_INT(SMALLINIT));
    }
    else {
      insert_assoc(self->assigns_table, car(car(tmp_assigns)),
                   cons(NODE_FROM_INT(SMALLINIT), Nil));
    }

    tmp_assigns = cdr(tmp_assigns);
  }
  self->init_assign = assigns;
}

/**Function********************************************************************

  Synopsis           [Replaces the init(*) := expressions for the
  current node.]

  Description        [Replaces the init(*) := expressions for the
  current node. ]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_init_assign_exprs(HrcNode_ptr self, node_ptr assigns)
{
  HRC_NODE_CHECK_INSTANCE(self);

  hrc_node_free_list_and_clear_assign_map(self, SMALLINIT);

  HrcNode_set_init_assign_exprs(self, assigns);
}

/**Function********************************************************************

  Synopsis           [Gets the init(*) := expressions for the current node.]

  Description        [Gets the init(*) := expressions for the current
  node. It is a list of implicitly conjoined assignments.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_init_assign_exprs(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->init_assign;
}

/**Function********************************************************************

  Synopsis           [Adds an init(*) := assignment to the current node.]

  Description        [Adds an init(*) := assignment to the current
  node. An assignment is an ASSIGN node that has as left child init(*)
  and as right child the assignment.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_init_assign_expr(HrcNode_ptr self, node_ptr assign)
{
  node_ptr ass;
  HRC_NODE_CHECK_INSTANCE(self);

  ass = find_assoc(self->assigns_table, car(assign));

  if (Nil != ass) {
    /* We can have both SMALLINIT and NEXT, but not INVAR and
       INIT/NEXT nor 2 INIT assigns */
    nusmv_assert(NODE_FROM_INT(NEXT) == car(ass));
  }
  else {
    insert_assoc(self->assigns_table, car(assign),
                 cons(NODE_FROM_INT(SMALLINIT), Nil));
  }

  self->init_assign = cons(assign, self->init_assign);
}


/**Function********************************************************************

  Synopsis           [Sets the INVAR expressions for the current node.]

  Description        [Sets the INVAR expressions for the current
  node. It is a list of implicitly conjoined expressions.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_invar_exprs(HrcNode_ptr self, node_ptr exprs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->invar_expr);
  self->invar_expr = exprs;
}

/**Function********************************************************************

  Synopsis           [Replaces the INVAR expressions for the current
  node.]

  Description        [Replaces the INVAR expressions for the current
  node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_invar_exprs(HrcNode_ptr self, node_ptr exprs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->invar_expr);

  HrcNode_set_invar_exprs(self, exprs);
}

/**Function********************************************************************

  Synopsis           [Sets the INVAR expressions for the current node.]

  Description        [Sets the INVAR expressions for the current
  node. It is a possibly empty list of implicitly conjoined expressions.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_invar_exprs(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->invar_expr;
}

/**Function********************************************************************

  Synopsis           [Adds an INVAR expression to the current node.]

  Description        [Adds an INVAR expression to the current node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_invar_expr(HrcNode_ptr self, node_ptr expr)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->invar_expr = cons(expr, self->invar_expr);
}

/**Function********************************************************************

  Synopsis           [Sets the (*) := expressions for the current node.]

  Description        [Sets the (*) := expressions for the current
  node. It is a list of implicitly conjoined assignments.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_invar_assign_exprs(HrcNode_ptr self, node_ptr assigns)
{
  HRC_NODE_CHECK_INSTANCE(self);

  node_ptr tmp_assigns;

  nusmv_assert(Nil == self->invar_assign);

  /* Avoids to rewrite assigns */
  tmp_assigns = assigns;
  while (Nil != tmp_assigns) {
    node_ptr ass = find_assoc(self->assigns_table, car(car(tmp_assigns)));

    if (Nil != ass) {
      /* We can have both SMALLINIT and NEXT, but not INVAR and
         INIT/NEXT nor 2 INVAR assigns */
      error_unreachable_code();
    }
    else {
      insert_assoc(self->assigns_table, car(car(tmp_assigns)),
                   cons(NODE_FROM_INT(INVAR), Nil));
    }

    tmp_assigns = cdr(tmp_assigns);
  }

  self->invar_assign = assigns;
}

/**Function********************************************************************

  Synopsis           [Replaces the (*) := expressions for the current node.]

  Description        [Replaces the (*) := expressions for the current
  node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_invar_assign_exprs(HrcNode_ptr self, node_ptr assigns)
{
  HRC_NODE_CHECK_INSTANCE(self);

  hrc_node_free_list_and_clear_assign_map(self, INVAR);
  /* FREE_CONS_LIST_AND_SET_TO_NIL(self->invar_assign); */

  HrcNode_set_invar_assign_exprs(self, assigns);
}

/**Function********************************************************************

  Synopsis           [Gets the (*) := expressions for the current node.]

  Description        [Gets the (*) := expressions for the current
  node. It is a list of implicitly conjoined assignments.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_invar_assign_exprs(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->invar_assign;
}

/**Function********************************************************************

  Synopsis           [Adds an (*) := assignment to the current node.]

  Description        [Adds an (*) := assignment to the current
  node. An assignment is an ASSIGN node that has as left child (*)
  and as right child the assignment.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_invar_assign_expr(HrcNode_ptr self, node_ptr assign)
{
  node_ptr ass;
  HRC_NODE_CHECK_INSTANCE(self);

  ass = find_assoc(self->assigns_table, car(assign));

  if (Nil != ass) {
    /* We can have both SMALLINIT and NEXT, but not INVAR and
       INIT/NEXT nor 2 INVAR assigns */
    error_unreachable_code();
  }
  else {
    insert_assoc(self->assigns_table, car(assign),
                 cons(NODE_FROM_INT(INVAR), Nil));
  }

  self->invar_assign = cons(assign, self->invar_assign);
}

/**Function********************************************************************

   Synopsis          [Checks if an assignment can be declared within the node]

  Description        [Checks if an assignment can be declared within the node.
                      If an INIT/NEXT assign is already declared for a symbol,
                      then only a NEXT/INIT assign can be declared. If an INVAR
                      assignment is already declared, then no other assignments
                      can be declared]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean HrcNode_can_declare_assign(HrcNode_ptr self, node_ptr symbol,
                                 int assign_type)
{
  node_ptr ass = find_assoc(self->assigns_table, symbol);

  if (Nil != ass) {
    /* Both INIT and NEXT have been already declared */
    if (Nil != cdr(ass)) return false;

    /* An assignment has been already declared, cannon declare an
       INVAR assignment in any case, or the already declared
       assignment is an INVAR one */
    if ((INVAR == assign_type) ||
        (NODE_FROM_INT(INVAR) == car(ass))) return false;

    /* Assignment of assign_type already declared. */
    if (car(ass) == NODE_FROM_INT(assign_type)) return false;
  }

  return true;
}

/**Function********************************************************************

  Synopsis           [Sets the TRANS expressions for the current node.]

  Description        [Sets the TRANS expressions for the current
  node. It is a list of implicitly conjoined expressions.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_trans_exprs(HrcNode_ptr self, node_ptr exprs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->next_expr);
  self->next_expr = exprs;
}

/**Function********************************************************************

  Synopsis           [Replaces the TRANS expressions for the current node.]

  Description        [Replaces the TRANS expressions for the current
  node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_trans_exprs(HrcNode_ptr self, node_ptr exprs)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->next_expr);

  HrcNode_set_trans_exprs(self, exprs);
}

/**Function********************************************************************

  Synopsis           [Gets the TRANS expressions for the current node.]

  Description        [Gets the TRANS expressions for the current
  node. It is a possibly empty list of implicitly conjoined expressions.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_trans_exprs(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->next_expr;
}

/**Function********************************************************************

  Synopsis           [Adds an TRANS expression to the current node.]

  Description        [Adds an TRANS expression to the current node.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_trans_expr(HrcNode_ptr self, node_ptr expr)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->next_expr = cons(expr, self->next_expr);
}

/**Function********************************************************************

  Synopsis           [Sets the next(*) := expressions for the current node.]

  Description        [Sets the next(*) := expressions for the current
  node. It is a list of implicitly conjoined assignments.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_next_assign_exprs(HrcNode_ptr self, node_ptr assigns)
{
  HRC_NODE_CHECK_INSTANCE(self);

  node_ptr tmp_assigns;

  nusmv_assert(Nil == self->next_assign);

  /* Avoids to rewrite assings */
  tmp_assigns = assigns;
  while (Nil != tmp_assigns) {
    node_ptr ass = find_assoc(self->assigns_table, car(car(tmp_assigns)));

    if (Nil != ass) {
      /* We can have both SMALLINIT and NEXT, but not INVAR and
         INIT/NEXT nor 2 NEXT assigns */
      nusmv_assert(NODE_FROM_INT(SMALLINIT) == car(ass));
      setcdr(ass, NODE_FROM_INT(NEXT));
    }
    else {
      insert_assoc(self->assigns_table, car(car(tmp_assigns)),
                   cons(NODE_FROM_INT(NEXT), Nil));
    }

    tmp_assigns = cdr(tmp_assigns);
  }
  self->next_assign = assigns;
}

/**Function********************************************************************

  Synopsis           [Replaces the next(*) := expressions for the current node.]

  Description        [Replaces the next(*) := expressions for the current
  node. It is a list of implicitly conjoined assignments.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_next_assign_exprs(HrcNode_ptr self, node_ptr assigns)
{
  HRC_NODE_CHECK_INSTANCE(self);

  hrc_node_free_list_and_clear_assign_map(self, NEXT);
  /* FREE_CONS_LIST_AND_SET_TO_NIL(self->next_assign); */

  HrcNode_set_next_assign_exprs(self, assigns);
}

/**Function********************************************************************

  Synopsis           [Gets the next(*) := expressions for the current node.]

  Description        [Gets the next(*) := expressions for the current
  node. It is a list of implicitly conjoined assignments.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_next_assign_exprs(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->next_assign;
}

/**Function********************************************************************

  Synopsis           [Adds an next(*) := assignment to the current node.]

  Description        [Adds an next(*) := assignment to the current
  node. An assignment is an ASSIGN node that has as left child next(*)
  and as right child the assignment.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_next_assign_expr(HrcNode_ptr self, node_ptr assign)
{
  node_ptr ass;
  HRC_NODE_CHECK_INSTANCE(self);

  ass = find_assoc(self->assigns_table, car(assign));

  if (Nil != ass) {
    /* We can have both SMALLINIT and NEXT, but not INVAR and
       INIT/NEXT nor 2 NEXT assigns */
    nusmv_assert(NODE_FROM_INT(SMALLINIT) == car(ass));
  }
  else {
    insert_assoc(self->assigns_table, car(assign),
                 cons(NODE_FROM_INT(NEXT), Nil));
  }

  self->next_assign = cons(assign, self->next_assign);
}

/**Function********************************************************************

  Synopsis           [Sets the list of JUSTICE constraints.]

  Description        [Sets the list of JUSTICE constraints.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_justice_exprs(HrcNode_ptr self, node_ptr justices)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->justice);
  self->justice = justices;
}

/**Function********************************************************************

  Synopsis           [Replaces the list of JUSTICE constraints.]

  Description        [Replaces the list of JUSTICE constraints.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_justice_exprs(HrcNode_ptr self, node_ptr justices)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->justice);

  HrcNode_set_justice_exprs(self, justices);
}

/**Function********************************************************************

  Synopsis           [Gets the list of JUSTICE constraints.]

  Description        [Gets the list of JUSTICE constraints.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_justice_exprs(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->justice;
}

/**Function********************************************************************

  Synopsis           [Adds a JUSTICE constraint.]

  Description        [Adds a JUSTICE constraint.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_justice_expr(HrcNode_ptr self, node_ptr justice)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->justice = cons(justice, self->justice);
}

/**Function********************************************************************

  Synopsis           [Sets the list of COMPASSION constraints.]

  Description        [Sets the list of COMPASSION constraints.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_compassion_exprs(HrcNode_ptr self, node_ptr compassions)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->compassion);
  self->compassion = compassions;
}

/**Function********************************************************************

  Synopsis           [Replaces the list of COMPASSION constraints.]

  Description        [Replaces the list of COMPASSION constraints.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_compassion_exprs(HrcNode_ptr self, node_ptr compassions)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->compassion);

  HrcNode_set_compassion_exprs(self, compassions);
}

/**Function********************************************************************

  Synopsis           [Gets the list of COMPASSION constraints.]

  Description        [Gets the list of COMPASSION constraints.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_compassion_exprs(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->compassion;
}

/**Function********************************************************************

  Synopsis           [Adds a COMPASSION constraint.]

  Description        [Adds a COMPASSION constraint.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_compassion_expr(HrcNode_ptr self, node_ptr compassion)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->compassion = cons(compassion, self->compassion);
}

/**Function********************************************************************

  Synopsis           [Sets the list of CONSTANTS declarations.]

  Description        [Sets the list of CONSTANTS declarations.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_constants(HrcNode_ptr self, node_ptr constants)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->constants);
  self->constants = constants;
}

/**Function********************************************************************

  Synopsis           [Replaces the list of CONSTANTS declarations.]

  Description        [Replaces the list of CONSTANTS declarations.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_constants(HrcNode_ptr self, node_ptr constants)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->constants);

  HrcNode_set_constants(self, constants);
}

/**Function********************************************************************

  Synopsis           [Gets the list of CONSTANTS declarations.]

  Description        [Gets the list of CONSTANTS declarations.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_constants(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->constants;
}

/**Function********************************************************************

  Synopsis           [Adds a CONSTANTS declaration to the list of
  constants.]

  Description        [Adds a CONSTANTS declaration to the list of
  constants. All constants are kept in a unique list.

  constant is a list of constants.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_constants(HrcNode_ptr self, node_ptr constant)
{
  HRC_NODE_CHECK_INSTANCE(self);

  while (Nil != constant) {
    nusmv_assert(CONS == node_get_type(constant));

    self->constants = cons(car(constant), self->constants);

    constant = cdr(constant);
  }
}

/**Function********************************************************************

  Synopsis           [Sets the list of CTL properties.]

  Description        [Sets the list of CTL properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_ctl_properties(HrcNode_ptr self, node_ptr ctls)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->ctl_props);
  self->ctl_props = ctls;
}

/**Function********************************************************************

  Synopsis           [Replaces the list of CTL properties.]

  Description        [Replaces the list of CTL properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_ctl_properties(HrcNode_ptr self, node_ptr ctls)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->ctl_props);

  HrcNode_set_ctl_properties(self, ctls);
}

/**Function********************************************************************

  Synopsis           [Gets the list of CTL properties.]

  Description        [Gets the list of CTL properties.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_ctl_properties(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->ctl_props;
}

/**Function********************************************************************

  Synopsis           [Adds a CTL property.]

  Description        [Adds a CTL property.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_ctl_property_expr(HrcNode_ptr self, node_ptr ctl)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->ctl_props = cons(ctl, self->ctl_props);
}

/**Function********************************************************************

  Synopsis           [Sets the list of LTL properties.]

  Description        [Sets the list of LTL properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_ltl_properties(HrcNode_ptr self, node_ptr ltls)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->ltl_props);
  self->ltl_props = ltls;
}

/**Function********************************************************************

  Synopsis           [Replaces the list of LTL properties.]

  Description        [Replaces the list of LTL properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_ltl_properties(HrcNode_ptr self, node_ptr ltls)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->ltl_props);

  HrcNode_set_ltl_properties(self, ltls);
}

/**Function********************************************************************

  Synopsis           [Gets the list of LTL properties.]

  Description        [Gets the list of LTL properties.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_ltl_properties(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->ltl_props;
}

/**Function********************************************************************

  Synopsis           [Adds an LTL property.]

  Description        [Adds an LTL property.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_ltl_property_expr(HrcNode_ptr self, node_ptr ltl)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->ltl_props = cons(ltl, self->ltl_props);
}


/**Function********************************************************************

  Synopsis           [Sets the list of PSL properties.]

  Description        [Sets the list of PSL properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_psl_properties(HrcNode_ptr self, node_ptr psls)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->psl_props);
  self->psl_props = psls;
}

/**Function********************************************************************

  Synopsis           [Replaces the list of PSL properties.]

  Description        [Replaces the list of PSL properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_psl_properties(HrcNode_ptr self, node_ptr psls)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->psl_props);

  HrcNode_set_psl_properties(self, psls);
}

/**Function********************************************************************

  Synopsis           [Gets the list of PSL properties.]

  Description        [Gets the list of PSL properties.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_psl_properties(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->psl_props;
}

/**Function********************************************************************

  Synopsis           [Adds an PSL property.]

  Description        [Adds an PSL property.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_psl_property_expr(HrcNode_ptr self, node_ptr psl)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->psl_props = cons(psl, self->psl_props);
}


/**Function********************************************************************

  Synopsis           [Sets the list of INVARIANT properties.]

  Description        [Sets the list of INVARIANT properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_invar_properties(HrcNode_ptr self, node_ptr invars)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->invar_props);
  self->invar_props = invars;
}

/**Function********************************************************************

  Synopsis           [Replaces the list of INVARIANT properties.]

  Description        [Replaces the list of INVARIANT properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_invar_properties(HrcNode_ptr self, node_ptr invars)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->invar_props);

  HrcNode_set_invar_properties(self, invars);
}

/**Function********************************************************************

  Synopsis           [Gets the list of INVARIANT properties.]

  Description        [Gets the list of INVARIANT properties.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_invar_properties(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->invar_props;
}

/**Function********************************************************************

  Synopsis           [Adds an INVARIANT property.]

  Description        [Adds an INVARIANT property.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_invar_property_expr(HrcNode_ptr self, node_ptr invar)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->invar_props = cons(invar, self->invar_props);
}


/**Function********************************************************************

  Synopsis           [Sets the list of COMPUTE properties.]

  Description        [Sets the list of COMPUTE properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_compute_properties(HrcNode_ptr self, node_ptr computes)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Nil == self->compute_props);
  self->compute_props = computes;
}

/**Function********************************************************************

  Synopsis           [Replaces the list of COMPUTE properties.]

  Description        [Replaces the list of COMPUTE properties.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_replace_compute_properties(HrcNode_ptr self, node_ptr computes)
{
  HRC_NODE_CHECK_INSTANCE(self);

  FREELIST_AND_SET_TO_NIL(self->compute_props);

  HrcNode_set_compute_properties(self, computes);
}

/**Function********************************************************************

  Synopsis           [Gets the list of COMPUTE properties.]

  Description        [Gets the list of COMPUTE properties.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
node_ptr HrcNode_get_compute_properties(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->compute_props;
}

/**Function********************************************************************

  Synopsis           [Adds a COMPUTE property.]

  Description        [Adds a COMPUTE property.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_compute_property_expr(HrcNode_ptr self, node_ptr compute)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->compute_props = cons(compute, self->compute_props);
}


/**Function********************************************************************

  Synopsis           [Sets the list of local childs for the current node.]

  Description        [Sets the list of local childs for the current
  node. Assumption is that the child nodes have the current node as parent.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_set_child_hrc_nodes(HrcNode_ptr self, Slist_ptr list)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(Slist_is_empty(self->childs));
  Slist_destroy(self->childs);

  {
    Siter iter;
    for(iter = Slist_first(list);
        (false == Siter_is_end(iter));
        iter = Siter_next(iter)) {
      nusmv_assert(HrcNode_get_parent(HRC_NODE(Siter_element(iter))) == self);
    }
  }

  self->childs = list;
}

/**Function********************************************************************

  Synopsis           [Gets the list of child nodes.]

  Description        [Gets the list of child nodes.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
/* Get the local list of child nodes for current node */
Slist_ptr HrcNode_get_child_hrc_nodes(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->childs;
}

/**Function********************************************************************

  Synopsis           [Adds a child node to the current node.]

  Description        [Add a child node to the current node. The parent
  of the child should have been set by someone else and it is expected
  to be the current one.]

  SideEffects        [Structure is updated]

  SeeAlso            [optional]

******************************************************************************/
void HrcNode_add_child_hrc_node(HrcNode_ptr self, HrcNode_ptr node)
{
  HRC_NODE_CHECK_INSTANCE(self);

  nusmv_assert(HrcNode_get_parent(node) == self);
  Slist_push(self->childs, (void *)node);
}


/**Function********************************************************************

  Synopsis           [Returns the pointer to a node instance of mod_type.]

  Description        [Returns the pointer to the first instance of a
  module of type mod_type encountered in a depth first traversal of
  the hierarchy tree. Returns HRC_NODE(NULL) if no instance exists.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
HrcNode_ptr HrcNode_find_hrc_node_by_mod_type(const HrcNode_ptr self,
                                              const node_ptr mod_type)
{
  HRC_NODE_CHECK_INSTANCE(self);

  if (self->name == mod_type) return self;
  else {
    Siter iter;
    HrcNode_ptr r = HRC_NODE(NULL);
    for (iter = Slist_first(self->childs);
         false == Siter_is_end(iter);
         iter = Siter_next(iter)) {
      r = HrcNode_find_hrc_node_by_mod_type((HrcNode_ptr)Siter_element(iter),
                                            mod_type);
      if (HRC_NODE(NULL) != r) break;
    }
    return r;
  }
}


/**Function********************************************************************

  Synopsis           [Returns the pointer to the a module instance of a
  of given name.]

  Description        [Returns the pointer to the a module instance of a
  of given name. Returns HRC_NODE(NULL) if no instance exists.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
HrcNode_ptr HrcNode_find_hrc_node_by_instance_name(const HrcNode_ptr self,
                                                   node_ptr name)
{
  HRC_NODE_CHECK_INSTANCE(self);

  if (node_normalize(self->instance_name) ==
      node_normalize(name)) return self;
  else {
    Siter iter;
    HrcNode_ptr r = HRC_NODE(NULL);
    for (iter = Slist_first(self->childs);
         false == Siter_is_end(iter);
         iter = Siter_next(iter)) {
      r = HrcNode_find_hrc_node_by_instance_name((HrcNode_ptr)Siter_element(iter),
                                                 name);
      if (HRC_NODE(NULL) != r) break;
    }
    return r;
  }
}


/**Function********************************************************************

  Synopsis           [Checks wether current node is the root of the hierarchy.]

  Description        [Checks wether current node is the root of the
  hierarchy. Returns true if it is the root, false otherwise.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
boolean HrcNode_is_root(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return (HRC_NODE(NULL) == self->parent);
}

/**Function********************************************************************

  Synopsis           [Checks wether current node is a leaf node in the hierarchy.]

  Description        [Checks wether current node is a leaf node in the
  hierarchy. Returns true if it is a leaf node, false otherwise.]

  SideEffects        [None]

  SeeAlso            [optional]

******************************************************************************/
boolean HrcNode_is_leaf(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return Slist_is_empty(self->childs);
}

/**Function********************************************************************

  Synopsis           [Creates a new node that is a copy of self without
  considering children.]

  Description        [Creates a new node that is a copy of self without
  considering children.

  The copy does not even link a node with its parent.]

  SideEffects        []

  SeeAlso            [HrcNode_copyRename]

******************************************************************************/
HrcNode_ptr HrcNode_copy(const HrcNode_ptr self)
{
  HrcNode_ptr hrc_copy;

  HRC_NODE_CHECK_INSTANCE(self);

  hrc_copy = HrcNode_create();

  hrc_copy->st = self->st;
  hrc_copy->lineno = self->lineno;
  hrc_copy->name = self->name;
  hrc_copy->instance_name = self->instance_name;

  /* parent is set to Nil! The copy is not linked to any node. */
  hrc_copy->parent = HRC_NODE(Nil);

  /* List that can be copied directly */
  hrc_copy->init_expr = copy_list(self->init_expr);
  hrc_copy->invar_expr = copy_list(self->invar_expr);
  hrc_copy->next_expr = copy_list(self->next_expr);
  hrc_copy->justice = copy_list(self->justice);
  hrc_copy->constants = copy_list(self->constants);
  hrc_copy->ctl_props = copy_list(self->ctl_props);
  hrc_copy->ltl_props = copy_list(self->ltl_props);
  hrc_copy->psl_props = copy_list(self->psl_props);
  hrc_copy->invar_props = copy_list(self->invar_props);
  hrc_copy->compute_props = copy_list(self->compute_props);

  /* List that must be deep copied, copying also list elements */
  hrc_copy->formal_parameters =
    hrc_node_copy_cons_list(self->formal_parameters);
  hrc_copy->actual_parameters =
    hrc_node_copy_cons_list(self->actual_parameters);
  hrc_copy->state_variables = hrc_node_copy_cons_list(self->state_variables);
  hrc_copy->input_variables = hrc_node_copy_cons_list(self->input_variables);
  hrc_copy->frozen_variables =
    hrc_node_copy_cons_list(self->frozen_variables);
  hrc_copy->defines = hrc_node_copy_cons_list(self->defines);
  hrc_copy->array_defines = hrc_node_copy_cons_list(self->array_defines);
  hrc_copy->init_assign = hrc_node_copy_cons_list(self->init_assign);
  hrc_copy->invar_assign = hrc_node_copy_cons_list(self->invar_assign);
  hrc_copy->next_assign = hrc_node_copy_cons_list(self->next_assign);
  hrc_copy->compassion = hrc_node_copy_cons_list(self->compassion);

  /* Copy the assign's table */
  {
    assoc_iter aiter;
    node_ptr sym;
    node_ptr val;

    ASSOC_FOREACH(self->assigns_table, aiter, &sym, &val) {
      insert_assoc(hrc_copy->assigns_table,
                   /* Use the same key, this is shared (find_node) */
                   sym,
                   /* Avoid memory sharing re-creating the cons node */
                   cons(car(val), cdr(val)));
    }
  }

  /* We use the same reference for the undef field of node */
  hrc_copy->undef = self->undef;

  return hrc_copy;
}

/**Function********************************************************************

  Synopsis           [Creates a new node that is a copy of self without
  considering children. The name of the module of the new node is
  set as new_module_name.]

  Description        [Creates a new node that is a copy of self without
  considering childre. The name of the module of the new node is
  set as new_module_name.

  The copy does not even link a node with its parent.]

  SideEffects        []

  SeeAlso            [HrcNode_copy]

******************************************************************************/
HrcNode_ptr HrcNode_copy_rename(const HrcNode_ptr self,
                                node_ptr new_module_name)
{
  HrcNode_ptr hrc_copy;

  hrc_copy = HrcNode_copy(self);

  hrc_copy->name = new_module_name;

  return hrc_copy;
}

/**Function********************************************************************

  Synopsis           [Getter for the undef field]

  Description        [Getter for the undef field]

  SideEffects        []

  SeeAlso            [HrcNode_set_undef]

******************************************************************************/
void* HrcNode_get_undef(const HrcNode_ptr self)
{
  HRC_NODE_CHECK_INSTANCE(self);

  return self->undef;
}


/**Function********************************************************************

  Synopsis           [Getter for the undef field]

  Description        [Getter for the undef field]

  SideEffects        []

  SeeAlso            [HrcNode_set_undef]

******************************************************************************/
void HrcNode_set_undef(const HrcNode_ptr self, void* undef)
{
  HRC_NODE_CHECK_INSTANCE(self);

  self->undef = undef;
}

/**Function********************************************************************

  Synopsis           [Creates a copy of self and recursively of all
  its children.]

  Description        [Creates a copy of self and recursively of all
  its children.

  This function is currently NOT IMPLEMENTED.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
HrcNode_ptr HrcNode_recursive_copy(const HrcNode_ptr self)
{
  HrcNode_ptr hrc_copy;
  Slist_ptr children;
  Siter iter;

  HRC_NODE_CHECK_INSTANCE(self);

  /* Copy the contents of the current node */
  hrc_copy = HrcNode_copy(self);

  /* Recursively copy children of node */
  children = Slist_copy_reversed(self->childs);
  SLIST_FOREACH(children, iter) {
    HrcNode_ptr child;
    HrcNode_ptr child_copy;

    child = HRC_NODE(Siter_element(iter));
    child_copy = HrcNode_recursive_copy(child);

    /* We must set the parent of child_copy */
    child_copy->parent = hrc_copy;

    HrcNode_add_child_hrc_node(hrc_copy, child_copy);
  }
  Slist_destroy(children);

  return hrc_copy;
}

/**Function********************************************************************

  Synopsis           [Returns the variable var_name of type var_type.]

  Description        [Returns the variable var_name of type
  var_type. The search is performed only inside self node, thus the
  function does not recur over hierarchy.
  Nil is returned if variable is not found.

  var_name is the name of the variable while type is the type of
  variable to search (VAR, FROZENVAR, INVAR).]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr HrcNode_find_var(HrcNode_ptr self, node_ptr var_name, int type)
{
  node_ptr var_list;
  node_ptr found_var;

  HRC_NODE_CHECK_INSTANCE(self);

  found_var = Nil;

  switch (type) {
  case VAR:
    var_list = HrcNode_get_state_variables(self);
    break;
  case FROZENVAR:
    var_list = HrcNode_get_frozen_variables(self);
    break;
  case IVAR:
    var_list = HrcNode_get_input_variables(self);
    break;
  default:
    internal_error("HrcNode: %d is not a valid variable type!", type);
  }

  /* Search normalizing node name! */
  var_name = node_normalize(var_name);

  while (Nil != var_list) {
    node_ptr var;
    var = car(var_list);

    nusmv_assert(Nil != var);
    /* var must have a name */
    nusmv_assert(Nil != car(var));

    if  (node_normalize(car(var)) == var_name) {
      found_var = var;
      break;
    }

    var_list = cdr(var_list);
  }

  return found_var;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The HrcNode class private initializer]

  Description        [The HrcNode class private initializer]

  SideEffects        []

  SeeAlso            [HrcNode_create]

******************************************************************************/
static void hrc_node_init(HrcNode_ptr self)
{
  /* members initialization */
  self->st = SYMB_TABLE(NULL);
  self->lineno = 0;
  self->name = Nil;
  self->instance_name = Nil;
  self->parent = HRC_NODE(NULL);
  self->formal_parameters = Nil;
  self->actual_parameters = Nil;
  self->state_variables = Nil;
  self->input_variables = Nil;
  self->frozen_variables = Nil;
  self->defines = Nil;
  self->array_defines = Nil;
  self->init_expr = Nil;
  self->init_assign = Nil;
  self->invar_expr = Nil;
  self->invar_assign = Nil;
  self->next_expr = Nil;
  self->next_assign = Nil;
  self->justice = Nil;
  self->compassion = Nil;
  self->constants = Nil;
  self->invar_props = Nil;
  self->ctl_props = Nil;
  self->ltl_props = Nil;
  self->psl_props = Nil;
  self->compute_props = Nil;
  self->childs = Slist_create();
  self->undef = (void*)NULL;
  self->assigns_table = new_assoc();
}


/**Function********************************************************************

  Synopsis           [The HrcNode class private deinitializer]

  Description        [The HrcNode class private deinitializer]

  SideEffects        []

  SeeAlso            [HrcNode_destroy]

******************************************************************************/
static void hrc_node_deinit(HrcNode_ptr self)
{
  /* members deinitialization */
  self->st = SYMB_TABLE(NULL);
  self->lineno = 0;
  self->name = Nil;
  self->instance_name = Nil;
  self->parent = HRC_NODE(NULL);

  FREE_CONS_LIST_AND_SET_TO_NIL(self->formal_parameters);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->actual_parameters);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->state_variables);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->input_variables);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->frozen_variables);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->defines);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->array_defines);
  FREELIST_AND_SET_TO_NIL(self->init_expr);
  FREELIST_AND_SET_TO_NIL(self->invar_expr);
  FREELIST_AND_SET_TO_NIL(self->next_expr);
  FREELIST_AND_SET_TO_NIL(self->justice);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->compassion);
  FREELIST_AND_SET_TO_NIL(self->constants);
  FREELIST_AND_SET_TO_NIL(self->ctl_props);
  FREELIST_AND_SET_TO_NIL(self->ltl_props);
  FREELIST_AND_SET_TO_NIL(self->psl_props);
  FREELIST_AND_SET_TO_NIL(self->invar_props);
  FREELIST_AND_SET_TO_NIL(self->compute_props);

  /* here the hrc_node_free_list_and_clear_assign_map is not used so
     we can avoid to pass 3 times through the hashmap, which is
     instead freed using the hrc_node_free_cons_map_fun */
  FREE_CONS_LIST_AND_SET_TO_NIL(self->init_assign);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->next_assign);
  FREE_CONS_LIST_AND_SET_TO_NIL(self->invar_assign);

  /* Free all cons nodes in the map values */
  clear_assoc_and_free_entries(self->assigns_table,
                               hrc_node_free_cons_map_fun);
  free_assoc(self->assigns_table);

  /* It is the responsibility of the creator to destroy the childrens. */
  Slist_destroy(self->childs);
  self->childs = SLIST(Nil);
  /* It is the responsibility of the creator to free this area before
     calling this function */
  if (self->undef != (void*)NULL) {
    self->undef = (void*)NULL;
  }

}

/**Function********************************************************************

  Synopsis           [Copy a list made of CONS elements.]

  Description        [Copy a list made of CONS elements.

  Also CONS elements are copied when copying the list. In this way the returned
  list can be used in a copy of the current node.

  New node are used to create cons elements. The copy preserves order of
  elements.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr hrc_node_copy_cons_list(node_ptr cons_list)
{
  node_ptr reversed_list;
  node_ptr new_list;

  reversed_list = Nil;

  while (Nil != cons_list) {
    node_ptr element;
    node_ptr new_element;

    element = car(cons_list);

    /* copy the new list */
    new_element = new_node(node_get_type(element), car(element), cdr(element));
    reversed_list = new_node(node_get_type(cons_list),
                             new_element,
                             reversed_list);

    cons_list = cdr(cons_list);
  }

  new_list = reverse_ns(reversed_list);
  free_list(reversed_list);

  return new_list;
}

/**Function********************************************************************

  Synopsis           [Frees all the elements contained in a CONS list.]

  Description        [Frees all the elements contained in a CONS list.]

  SideEffects        [Elements contained in list are freed.]

  SeeAlso            []

******************************************************************************/
static void hrc_node_free_cons_elements_in_list_and_list(node_ptr list)
{
  while (NODE_PTR(Nil) != list) {
    node_ptr element;

    element = car(list);

    if (NODE_PTR(Nil) != element) {
      free_node(element);
    }

    element = list;
    list = cdr(list);
    free_node(element);
  }
}

/**Function********************************************************************

  Synopsis           [Frees all the elements contained in a ASSIGN list. and
                      removes entries from the assign map]

  Description        [Frees all the elements contained in a ASSIGN list. and
                      removes entries from the assign map]

  SideEffects        [Elements contained in list are freed, the list is
                      freed and set to Nil into the HrcNode]

  SeeAlso            []

******************************************************************************/
static void hrc_node_free_list_and_clear_assign_map(HrcNode_ptr self,
                                                    int assign_type)
{
  node_ptr list;
  switch (assign_type) {
  case SMALLINIT:
    list = self->init_assign;
    self->init_assign = Nil;
    break;
  case NEXT:
    list = self->next_assign;
    self->next_assign = Nil;
    break;
  case INVAR:
    list = self->invar_assign;
    self->invar_assign = Nil;
    break;
  default:
    error_unreachable_code();
  }

  while (Nil != list) {
                                   /* we just need the symb name */
    node_ptr tmp = find_assoc(self->assigns_table, car(car(list)));

    /* The assignment must exist */
    nusmv_assert(Nil != tmp);

    /* This assigment is in the car part of the node */
    if (NODE_FROM_INT(assign_type) == car(tmp)) {
      /* If this is the only assignment for this symbol, we can remove
         the node from the hash and free it */
      if (Nil == cdr(tmp)) {
        remove_assoc(self->assigns_table, car(car(list)));
        free_node(tmp);
      }
      /* Otherwise shift cdr to car */
      else {
        setcar(tmp, cdr(tmp));
        setcdr(tmp, Nil);
      }
    }
    /* Otherwise it must be in the right part, which will be set to
       Nil */
    else {
      nusmv_assert(Nil != cdr(tmp) &&
                   NODE_FROM_INT(assign_type) == cdr(tmp));
      setcdr(tmp, Nil);
    }

    tmp = list;
    list = cdr(list);
    /* Free the ASSIGN node */
    free_node(car(tmp));
    /* Free the CONS node */
    free_node(tmp);
  }
}

/**Function********************************************************************

  Synopsis           [Function for freeing cons nodes into a map]

  Description        [Function for freeing cons nodes into a map]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static assoc_retval hrc_node_free_cons_map_fun(char *key,
                                               char *data,
                                               char *arg)
{
  if ((char*)NULL != data) {
    free_node(NODE_PTR(data));
  }
  return ASSOC_DELETE;
}

/**AutomaticEnd***************************************************************/
