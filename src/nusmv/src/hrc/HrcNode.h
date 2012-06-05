/**CHeaderFile*****************************************************************

  FileName    [HrcNode.h]

  PackageName [hrc]

  Synopsis    [Public interface of class 'HrcNode']

  Description []

  SeeAlso     [HrcNode.c]

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

  Revision    [$Id: HrcNode.h,v 1.1.2.10 2009-12-14 13:41:52 nusmv Exp $]

******************************************************************************/


#ifndef __HRC_NODE_H__
#define __HRC_NODE_H__

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

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class HrcNode]

  Description []

******************************************************************************/
typedef struct HrcNode_TAG*  HrcNode_ptr;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/* Global variable declaration of hrc node that represents main
   module. */
EXTERN HrcNode_ptr mainHrcNode;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class HrcNode]

  Description [These macros must be used respectively to cast and to check
  instances of class HrcNode]

******************************************************************************/
#define HRC_NODE(self) \
         ((HrcNode_ptr) self)

#define HRC_NODE_CHECK_INSTANCE(self) \
         (nusmv_assert(HRC_NODE(self) != HRC_NODE(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* Create an empty Hrc Node and returns a pointer to it */
EXTERN HrcNode_ptr HrcNode_create ARGS((void));
/* Deallocate a previously allocated Hrc Node */
EXTERN void HrcNode_destroy ARGS((HrcNode_ptr self));
/* Deallocate a previously allocated Hrc Node, recursing on
   deallocating also the childs */
EXTERN void HrcNode_destroy_recur ARGS((HrcNode_ptr self));

EXTERN void HrcNode_cleanup ARGS((HrcNode_ptr self));

/* Set the Symbol Table */
EXTERN void HrcNode_set_symbol_table ARGS((HrcNode_ptr self, SymbTable_ptr st));
/* Get the Symbol Table */
SymbTable_ptr HrcNode_get_symbol_table ARGS((HrcNode_ptr self));

/* Set the line number which this set is defined at */
EXTERN void HrcNode_set_lineno ARGS((HrcNode_ptr self, int lineno));
/* Get the line number of the node, as previously set wuth SetLineno */
EXTERN int HrcNode_get_lineno ARGS((const HrcNode_ptr self));

/* Set the name of the current node */
EXTERN void HrcNode_set_name ARGS((HrcNode_ptr self, node_ptr name));
/* Get the normalized name of the current node */
EXTERN node_ptr HrcNode_get_name ARGS((const HrcNode_ptr self));
/* Get the name of the current node, NOT normalized and as passed to SetName */
EXTERN node_ptr HrcNode_get_crude_name ARGS((const HrcNode_ptr self));

/* Set the instance name of the current node */
EXTERN void HrcNode_set_instance_name ARGS((HrcNode_ptr self, node_ptr name));
/* Get the instance name of the current node */
EXTERN node_ptr HrcNode_get_instance_name ARGS((const HrcNode_ptr self));
/* Get the flattened instance name of the current node */
EXTERN node_ptr HrcNode_get_flattened_instance_name ARGS((const HrcNode_ptr self));

/* Set the father of the current node */
EXTERN void HrcNode_set_parent ARGS((const HrcNode_ptr self, HrcNode_ptr father));
/* Get the father of the current node, NULL if it is the root */
EXTERN HrcNode_ptr HrcNode_get_parent ARGS((const HrcNode_ptr self));

/* Set the formal parameters of the current node */
EXTERN void HrcNode_set_formal_parameters ARGS((HrcNode_ptr self, node_ptr par));
/* Replace the formal parameters of the current node */
EXTERN void HrcNode_replace_formal_parameters ARGS((HrcNode_ptr self, node_ptr par));
/* Get the formal parameters of the current node */
EXTERN node_ptr HrcNode_get_formal_parameters ARGS((const HrcNode_ptr self));
/* Add a formal parameters of the current node */
EXTERN void HrcNode_add_formal_parameter ARGS((HrcNode_ptr self, node_ptr par));

/* Set the actual parameters of the current node */
EXTERN void HrcNode_set_actual_parameters ARGS((HrcNode_ptr self, node_ptr par));
/* Replace the actual parameters of the current node */
EXTERN void HrcNode_replace_actual_parameters ARGS((HrcNode_ptr self, node_ptr par));
/* Get the actual parameters of the current node */
EXTERN node_ptr HrcNode_get_actual_parameters ARGS((const HrcNode_ptr self));
/* Add an actual parameters of the current node */
EXTERN void HrcNode_add_actual_parameter ARGS((HrcNode_ptr self, node_ptr par));

/* Set the local state variables for current node */
EXTERN void HrcNode_set_state_variables ARGS((HrcNode_ptr self, node_ptr vars));
/* Replace the local state variables for current node */
EXTERN void HrcNode_replace_state_variables ARGS((HrcNode_ptr self, node_ptr vars));
/* Get the local state variables for current node */
EXTERN node_ptr HrcNode_get_state_variables ARGS((const HrcNode_ptr self));
/* Add a state variable to the current node */
EXTERN void HrcNode_add_state_variable ARGS((HrcNode_ptr self, node_ptr var));

/* Set the local input variables for current node */
EXTERN void HrcNode_set_input_variables ARGS((HrcNode_ptr self, node_ptr vars));
/* Replace the local input variables for current node */
EXTERN void HrcNode_replace_input_variables ARGS((HrcNode_ptr self, node_ptr vars));
/* Get the local input variables for current node */
EXTERN node_ptr HrcNode_get_input_variables ARGS((const HrcNode_ptr self));
/* Add a state variable to the current node */
EXTERN void HrcNode_add_input_variable ARGS((HrcNode_ptr self, node_ptr var));

/* Set the local frozen variables for current node */
EXTERN void HrcNode_set_frozen_variables ARGS((HrcNode_ptr self, node_ptr vars));
/* Replace the local frozen variables for current node */
EXTERN void HrcNode_replace_frozen_variables ARGS((HrcNode_ptr self, node_ptr vars));
/* Get the local frozen variables for current node */
EXTERN node_ptr HrcNode_get_frozen_variables ARGS((const HrcNode_ptr self));
/* Add a state variable to the current node */
EXTERN void HrcNode_add_frozen_variable ARGS((HrcNode_ptr self, node_ptr var));

/* Set the local DEFINEs for current node */
EXTERN void HrcNode_set_defines ARGS((HrcNode_ptr self, node_ptr defs));
/* Replace the local DEFINEs for current node */
EXTERN void HrcNode_replace_defines ARGS((HrcNode_ptr self, node_ptr defs));
/* Get the local DEFINEs variables for current node */
EXTERN node_ptr HrcNode_get_defines ARGS((const HrcNode_ptr self));
/* Add a DEFINE symbol to the current node */
EXTERN void HrcNode_add_define ARGS((HrcNode_ptr self, node_ptr def));

/* Set the local MDEFINEs for current node */
EXTERN void HrcNode_set_array_defines ARGS((HrcNode_ptr self, node_ptr mdefs));
/* Replace the local MDEFINEs for current node */
EXTERN void HrcNode_replace_array_defines ARGS((HrcNode_ptr self, node_ptr mdefs));
/* Get the local MDEFINEs for the current node */
EXTERN node_ptr HrcNode_get_array_defines ARGS((const HrcNode_ptr self));
/* Add an MDEFINE symbol to the current node */
EXTERN void HrcNode_add_array_define ARGS((HrcNode_ptr self, node_ptr mdef));

/* Set the local INIT expression for current node */
EXTERN void HrcNode_set_init_exprs ARGS((HrcNode_ptr self, node_ptr exprs));
/* Replace the local INIT expression for current node */
EXTERN void HrcNode_replace_init_exprs ARGS((HrcNode_ptr self, node_ptr exprs));
/* Get the local INIT expression for current node */
EXTERN node_ptr HrcNode_get_init_exprs ARGS((const HrcNode_ptr self));
/* Add an INIT expression in conjunction to the current node */
EXTERN void HrcNode_add_init_expr ARGS((HrcNode_ptr self, node_ptr expr));

/* Set the local init(x) := expr expressions for current node */
EXTERN void HrcNode_set_init_assign_exprs ARGS((HrcNode_ptr self, node_ptr assigns));
/* Replace the local init(x) := expr expressions for current node */
EXTERN void HrcNode_replace_init_assign_exprs ARGS((HrcNode_ptr self, node_ptr assigns));
/* Get the local init(x) := expr expression for current node */
EXTERN node_ptr HrcNode_get_init_assign_exprs ARGS((const HrcNode_ptr self));
/* Add an init(x) := expr expression in conjunction to the current node */
EXTERN void HrcNode_add_init_assign_expr ARGS((HrcNode_ptr self, node_ptr assign));

/* Set the local INVAR expression for current node */
EXTERN void HrcNode_set_invar_exprs ARGS((HrcNode_ptr self, node_ptr exprs));
/* Replace the local INVAR expression for current node */
EXTERN void HrcNode_replace_invar_exprs ARGS((HrcNode_ptr self, node_ptr exprs));
/* Get the local INVAR expression for current node */
EXTERN node_ptr HrcNode_get_invar_exprs ARGS((const HrcNode_ptr self));
/* Add an INVAR expression in conjunction to the current node */
EXTERN void HrcNode_add_invar_expr ARGS((HrcNode_ptr self, node_ptr expr));

/* Set the local x := expr expressions for current node */
EXTERN void HrcNode_set_invar_assign_exprs ARGS((HrcNode_ptr self, node_ptr assigns));
/* Replace the local x := expr expressions for current node */
EXTERN void HrcNode_replace_invar_assign_exprs ARGS((HrcNode_ptr self, node_ptr assigns));
/* Get the local x := expr expression for current node */
EXTERN node_ptr HrcNode_get_invar_assign_exprs ARGS((const HrcNode_ptr self));
/* Add an x := expr expression in conjunction to the current node */
EXTERN void HrcNode_add_invar_assign_expr ARGS((HrcNode_ptr self, node_ptr assign));

/* Checks if an assignment can be added to the node */
boolean HrcNode_can_declare_assign ARGS((HrcNode_ptr self, node_ptr symbol,
                                       int assign_type));

/* Set the local TRANS expression for current node */
EXTERN void HrcNode_set_trans_exprs ARGS((HrcNode_ptr self, node_ptr exprs));
/* Replace the local TRANS expression for current node */
EXTERN void HrcNode_replace_trans_exprs ARGS((HrcNode_ptr self, node_ptr exprs));
/* Get the local TRANS expression for current node */
EXTERN node_ptr HrcNode_get_trans_exprs ARGS((const HrcNode_ptr self));
/* Add an TRANS expression in conjunction to the current node */
EXTERN void HrcNode_add_trans_expr ARGS((HrcNode_ptr self, node_ptr expr));

/* Set the local next(x) := epxr expression for current node */
EXTERN void HrcNode_set_next_assign_exprs ARGS((HrcNode_ptr self, node_ptr assigns));
/* Replace the local next(x) := epxr expression for current node */
EXTERN void HrcNode_replace_next_assign_exprs ARGS((HrcNode_ptr self, node_ptr assigns));
/* Get the local next(x) := epxr expression for current node */
EXTERN node_ptr HrcNode_get_next_assign_exprs ARGS((const HrcNode_ptr self));
/* Add an next(x) := epxr expression in conjunction to the current node */
EXTERN void HrcNode_add_next_assign_expr ARGS((HrcNode_ptr self, node_ptr assign));

/* Set the local JUSTICE epxr expression for current node */
EXTERN void HrcNode_set_justice_exprs ARGS((HrcNode_ptr self, node_ptr justices));
/* Replace the local JUSTICE epxr expression for current node */
EXTERN void HrcNode_replace_justice_exprs ARGS((HrcNode_ptr self, node_ptr justices));
/* Get the local JUSTICE epxr expression for current node */
EXTERN node_ptr HrcNode_get_justice_exprs ARGS((const HrcNode_ptr self));
/* Add an JUSTICE epxr expression in conjunction to the current node */
EXTERN void HrcNode_add_justice_expr ARGS((HrcNode_ptr self, node_ptr justice));

/* Set the local COMPASSION epxr expression for current node */
EXTERN void HrcNode_set_compassion_exprs ARGS((HrcNode_ptr self, node_ptr compassions));
/* Replace the local COMPASSION epxr expression for current node */
EXTERN void HrcNode_replace_compassion_exprs ARGS((HrcNode_ptr self, node_ptr compassions));
/* Get the local COMPASSION epxr expression for current node */
EXTERN node_ptr HrcNode_get_compassion_exprs ARGS((const HrcNode_ptr self));
/* Add an COMPASSION epxr expression in conjunction to the current node */
EXTERN void HrcNode_add_compassion_expr ARGS((HrcNode_ptr self, node_ptr compassion));

/* Set the local CONSTANTS declarations for current node */
EXTERN void HrcNode_set_constants ARGS((HrcNode_ptr self, node_ptr constants));
/* Replace the local CONSTANTS declarations for current node */
EXTERN void HrcNode_replace_constants ARGS((HrcNode_ptr self, node_ptr constants));
/* Get the local CONSTANTS declarations for current node */
EXTERN node_ptr HrcNode_get_constants ARGS((const HrcNode_ptr self));
/* Add the CONSTANT declaration to the list of CONSTANTS of this node */
EXTERN void HrcNode_add_constants ARGS((HrcNode_ptr self, node_ptr constant));

/* Set the local CTLSPEC epxr expression for current node */
EXTERN void HrcNode_set_ctl_properties ARGS((HrcNode_ptr self, node_ptr ctls));
/* Replace the local CTLSPEC epxr expression for current node */
EXTERN void HrcNode_replace_ctl_properties ARGS((HrcNode_ptr self, node_ptr ctls));
/* Get the local CTLSPEC epxr expression for current node */
EXTERN node_ptr HrcNode_get_ctl_properties ARGS((const HrcNode_ptr self));
/* Add an CTLSPEC epxr expression in conjunction to the current node */
EXTERN void HrcNode_add_ctl_property_expr ARGS((HrcNode_ptr self, node_ptr ctl));

/* Set the local LTLSPEC epxr expression for current node */
EXTERN void HrcNode_set_ltl_properties ARGS((HrcNode_ptr self, node_ptr ltls));
/* Replace the local LTLSPEC epxr expression for current node */
EXTERN void HrcNode_replace_ltl_properties ARGS((HrcNode_ptr self, node_ptr ltls));
/* Get the local LTLSPEC epxr expression for current node */
EXTERN node_ptr HrcNode_get_ltl_properties ARGS((const HrcNode_ptr self));
/* Add an LTLSPEC epxr expression in conjunction to the current node */
EXTERN void HrcNode_add_ltl_property_expr ARGS((HrcNode_ptr self, node_ptr ltl));

/* Set the local PSLSPEC epxr expression for current node */
EXTERN void HrcNode_set_psl_properties ARGS((HrcNode_ptr self, node_ptr psls));
/* Replace the local PSLSPEC epxr expression for current node */
EXTERN void HrcNode_replace_psl_properties ARGS((HrcNode_ptr self, node_ptr psls));
/* Get the local PSLSPEC epxr expression for current node */
EXTERN node_ptr HrcNode_get_psl_properties ARGS((const HrcNode_ptr self));
/* Add an PSLSPEC epxr expression in conjunction to the current node */
EXTERN void HrcNode_add_psl_property_expr ARGS((HrcNode_ptr self, node_ptr psl));

/* Set the local INVARSPEC epxr expression for current node */
EXTERN void HrcNode_set_invar_properties ARGS((HrcNode_ptr self, node_ptr invars));
/* Replace the local INVARSPEC epxr expression for current node */
EXTERN void HrcNode_replace_invar_properties ARGS((HrcNode_ptr self, node_ptr invars));
/* Get the local INVARSPEC epxr expression for current node */
EXTERN node_ptr HrcNode_get_invar_properties ARGS((const HrcNode_ptr self));
/* Add an INVARSPEC epxr expression in conjunction to the current node */
EXTERN void HrcNode_add_invar_property_expr ARGS((HrcNode_ptr self, node_ptr invar));

/* Set the local COMPUTESPEC epxr expression for current node */
EXTERN void HrcNode_set_compute_properties ARGS((HrcNode_ptr self, node_ptr computes));
/* Replace the local COMPUTESPEC epxr expression for current node */
EXTERN void HrcNode_replace_compute_properties ARGS((HrcNode_ptr self, node_ptr computes));
/* Get the local COMPUTESPEC epxr expression for current node */
EXTERN node_ptr HrcNode_get_compute_properties ARGS((const HrcNode_ptr self));
/* Add an COMPUTESPEC epxr expression in conjunction to the current node */
EXTERN void HrcNode_add_compute_property_expr ARGS((HrcNode_ptr self, node_ptr compute));

EXTERN void HrcNode_set_undef ARGS((const HrcNode_ptr self, void* undef));
EXTERN void* HrcNode_get_undef ARGS((const HrcNode_ptr self));

/* Set the local child nodes for current node. We assume the father of
   the child has been set by someone else */
EXTERN void HrcNode_set_child_hrc_nodes ARGS((HrcNode_ptr self, Slist_ptr list));
/* Get the local list of child nodes for current node */
EXTERN Slist_ptr HrcNode_get_child_hrc_nodes ARGS((const HrcNode_ptr self));
/* Add a child node to the current node. We assume the father has been
   set by someone else */
EXTERN void HrcNode_add_child_hrc_node ARGS((HrcNode_ptr self, HrcNode_ptr node));

/* Returns a pointer to a node in the hiearchy whose name is
   mod_type. NULL if it does not exixts */
EXTERN HrcNode_ptr
HrcNode_find_hrc_node_by_mod_type ARGS((const HrcNode_ptr self,
                                        node_ptr mod_type));

/* Returns a pointer to a node in the hiearchy whose name is
   name. NULL if it does not exits */
EXTERN HrcNode_ptr
HrcNode_find_hrc_node_by_instance_name ARGS((const HrcNode_ptr self,
                                             node_ptr name));

/* returns true if the node is the root */
EXTERN boolean HrcNode_is_root ARGS((const HrcNode_ptr self));

/* returns true if the node is a leaf node */
EXTERN boolean HrcNode_is_leaf ARGS((const HrcNode_ptr self));

/* Returns a copy of the HrcNode self */
EXTERN HrcNode_ptr HrcNode_copy ARGS((const HrcNode_ptr self));

/* Returns a copy of the HrcNode self setting its name to instance_name */
EXTERN HrcNode_ptr HrcNode_copy_rename ARGS((const HrcNode_ptr self,
                                            node_ptr new_module_name));

/* Returns a copy of the entire hierarchy that starts from HrcNode self */
EXTERN HrcNode_ptr HrcNode_recursive_copy ARGS((const HrcNode_ptr self));

/* Find var_name of a given type (state, frozen, input) in self node */
EXTERN node_ptr HrcNode_find_var ARGS((HrcNode_ptr self, node_ptr var_name, int type));

/**AutomaticEnd***************************************************************/

#endif /* __HRC_NODE_H__ */
