/**CFile***********************************************************************

  FileName    [hrcPrefixUtils.c]

  PackageName [hrc]

  Synopsis    [Utility functions used to concatenate/remove prefixes from
  names.]

  Description [Utility functions used to concatenate/remove prefixes from
  names.]

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
#include "hrcPrefixUtils.h"
#include "parser/symbols.h"
#include "utils/ustring.h"

static char rcsid[] UTIL_UNUSED = "$Id: hrcPrefixUtils.c,v 1.1.2.3 2009-11-03 11:29:18 nusmv Exp $";

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

  Synopsis           [Given a set of symbol returns a new set that
  contains only symbols that have a given prefix.]

  Description        [Given a set of symbol returns a new set that
  contains only symbols that have a given prefix.

  The returned set must be destroyed by the caller.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t hrc_prefix_utils_get_prefix_symbols(Set_t symbol_set,
                                          node_ptr prefix)
{
  Set_t result_set;
  Set_Iterator_t iter;

  result_set = Set_MakeEmpty();

  SET_FOREACH(symbol_set, iter) {
    node_ptr symbol;

    symbol = NODE_PTR(Set_GetMember(symbol_set, iter));

    if (NODE_PTR(Nil) != symbol) {
      if (hrc_prefix_utils_is_subprefix(prefix, car(symbol))) {
        result_set = Set_AddMember(result_set, symbol);
      }
    }
  }

  return result_set;
}

/**Function********************************************************************

  Synopsis           [Returns true if subprefix is contained in prefix.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean hrc_prefix_utils_is_subprefix(node_ptr subprefix, node_ptr prefix)
{
  if (subprefix == prefix) {
    /* subprefix and prefix are the same */
    return true;
  } else if (NODE_PTR(Nil) == prefix) {
    /* prefix is Nil while subprefix is not Nil */
    return false;
  } else {
    /* recursive call */
    return hrc_prefix_utils_is_subprefix(subprefix, car(prefix));
  }
}

/**Function********************************************************************

  Synopsis           [Build the expression prefixed by context.]

  Description        [Build the expression prefixed by context.

  If expression is of DOT or CONTEXT type we cannot build the tree
  DOT(context, expression). We need to recursively visit expression
  to build a correct DOT tree.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr hrc_prefix_utils_add_context(node_ptr context, node_ptr expression)
{
  node_ptr return_expression;

  nusmv_assert(NODE_PTR(Nil) != expression);

  if (DOT != node_get_type(expression) &&
      CONTEXT != node_get_type(expression)) {
    /* base case: concatenate context and expression */
    return_expression = find_node(DOT,
                                  context,
                                  find_atom(expression));
  } else {
    /* Recursively build the context with car(expression) and prefix
       it to cdr(expression) */
    context = hrc_prefix_utils_add_context(context,
                                       car(expression));
    return_expression = find_node(DOT,
                                  context,
                                  find_atom(cdr(expression)));
  }

  return return_expression;
}

/**Function********************************************************************

  Synopsis           [Get the first subcontext of the given symbol.]

  Description        [Get the first subcontext of the given symbol.

  Search the second CONTEXT or DOT node in symbol and returns it. If it
  is not found then Nil is returned.

  DOT and CONTEXT nodes are always searched in the car node.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr hrc_prefix_utils_get_first_subcontext(node_ptr symbol)
{
  node_ptr context;

  context = symbol;

  /* Search the first DOT or context node following the leftmost path
     of symbol.

     This is needed to get rid of nodes that can appear on top of DOT,
     like ARRAYOF.

     From this loop context will be a Nil node, a DOT node or a
     CONTEXT node.
  */
  while (NODE_PTR(Nil) != context &&
         DOT != node_get_type(context) &&
         CONTEXT != node_get_type(context)) {
    context = car(context);
  }

  /* Now context contains NIL or the first context of symbol.

     Get the first subcontext if it exsists.
   */
  if (NODE_PTR(Nil) != context) {
    /* get the first subcontext */
    if (NODE_PTR(Nil) == car(context)) {
      context = NODE_PTR(Nil);
    } else if (DOT == node_get_type(context) ||
               CONTEXT == node_get_type(context)) {
      /* found the subcontext */
      context = car(context);
    } else {
      /* subcontext does not exists. This should not happen! */
      context = NODE_PTR(Nil);
    }
  }

  return context;
}

/**Function********************************************************************

  Synopsis           [Removes context from identifier.]

  Description        [Removes context from identifier.
  If context is not ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr hrc_prefix_utils_remove_context(node_ptr identifier,
                                         node_ptr context)
{
  node_ptr identifier_no_context;

  if (Nil == identifier) {
    identifier_no_context = Nil;
  } else if (car(identifier) == context) {
    identifier_no_context = cdr(identifier);
  } else {
    node_ptr new_context;

    new_context = hrc_prefix_utils_remove_context(car(identifier), context);
    identifier_no_context = find_node(DOT, new_context, cdr(identifier));
  }

  return identifier_no_context;
}

/**Function********************************************************************

  Synopsis           [Creates a new name for the module instance.]

  Description        [Creates a new name for the module instance.

  The generated module name is <module_name>_<module_instance_flattened_name>]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr hrc_prefix_utils_assign_module_name(HrcNode_ptr instance,
                                             node_ptr instance_name)
{
  node_ptr module_name;
  node_ptr new_name;
  char* module_name_chr;
  char* instance_name_chr;
  char* new_name_chr;
  char* tmp;
  boolean found_double_quote;


  module_name = HrcNode_get_name(instance);

  /* Get char* representation of module and instance name */
  module_name_chr = sprint_node(module_name);
  instance_name_chr = sprint_node(instance_name);

  /* creates the new module name */
  new_name_chr = ALLOC(char, 
                       strlen(module_name_chr) +
                       strlen(instance_name_chr) +
                       2);
  
  if (NODE_PTR(Nil) == instance_name) {
    /* main module */
    sprintf(new_name_chr, "%s", module_name_chr);
  } else {
    sprintf(new_name_chr,
            "%s_%s",             
            module_name_chr, 
            instance_name_chr);
  }
  
  found_double_quote = false;

  /* substitutes . with _ inside the module name.
     substitutes " with _ inside the module name.
   */
  for (tmp = new_name_chr; *tmp != 0; tmp++) {
    if ('.' == *tmp) *tmp = '_';
    if ('"' == *tmp) {
      found_double_quote = true;
      *tmp = '_';
    }
  }

  /* manages double quote */
  if (found_double_quote) {
    char* copy_str;
    char* app;
    
    copy_str = ALLOC(char, strlen(new_name_chr) + 3);
    
    /* copy in copy_str new_name_chr, leaving free the first character.
       Pay attention that this operation does not copy terminator.       
     */
    strncpy((copy_str+1) , new_name_chr, strlen(new_name_chr));
    copy_str[0] = '"';
    /* Add ending double quote */
    copy_str[strlen(copy_str)-1] = '"';
    copy_str[strlen(copy_str)] = '\0';

    /* replace new_name_chr with copy_str */
    app = new_name_chr;
    new_name_chr = copy_str;
    FREE(app);    
  }

  /* Creates the new module name */
  new_name = find_node(ATOM, (node_ptr) find_string(new_name_chr), Nil);
  
  FREE(module_name_chr);
  FREE(instance_name_chr);
  FREE(new_name_chr);  
  
  return new_name;
}

/**Function********************************************************************

  Synopsis           [Given an instance returns its flattened name.]

  Description        [Given an instance returns its flattened name.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr hrc_prefix_utils_flatten_instance_name(HrcNode_ptr instance)
{
  node_ptr flattened_name;
  HrcNode_ptr instance_iter;
  Slist_ptr variables_chain;
  Siter variables_iter;

  variables_chain = Slist_create();

  /* The hierarchy is visited upward until the root node. 
     A stack of variables is created. At the top of the stack there should 
     be the main module.
   */
  instance_iter = instance;
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
  SLIST_FOREACH(variables_chain, variables_iter) {
    node_ptr current_var;

    current_var = ((node_ptr) Siter_element(variables_iter));
    if (NODE_PTR(Nil) != current_var) {
      /* the instance is not the main module.
         find_node is used to have an unique representation of the 
         variable name.
       */     
      flattened_name = find_node(DOT, 
                                 flattened_name, 
                                 find_atom(current_var));      
    }    
  }
  
  Slist_destroy(variables_chain);

  return flattened_name;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

