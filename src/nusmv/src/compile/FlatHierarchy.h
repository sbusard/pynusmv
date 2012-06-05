/**CHeaderFile*****************************************************************

  FileName    [FlatHierarchy.h]

  PackageName [compile]

  Synopsis [The class is used to store results of flattening a hierarchy.]

  Description [This class is virtually a set of fields to store
  various structures obtained after flattening parsed tree
  (i.e. module "main"). For example, there are list of INVARSPEC, a
  list of INIT expressions, a list of COMPASSION expressions, etc.

  Also this structure has a hash table to associate
  1. a variable on the left handside of an assignment to its right handside.
  2. a variable name to all constrains (INIT, TRANS, INVAR) which constain
     the given variable.

  See FlatHierarchy_create for more info on this class.]
  
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

#ifndef __FLAT_HIERARCHY_H__
#define __FLAT_HIERARCHY_H__

#include "node/node.h"
#include "set/set.h"
#include "compile/symb_table/SymbTable.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Typedef********************************************************************

  Synopsis    [The FlatHierarchy type]

  Description [The struct store info of flattened modules]

******************************************************************************/
typedef struct FlatHierarchy* FlatHierarchy_ptr;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


#define FLAT_HIERARCHY(x) ((FlatHierarchy_ptr) x)

#define FLAT_HIERARCHY_CHECK_INSTANCE(x) \
         ( nusmv_assert(FLAT_HIERARCHY(x) != FLAT_HIERARCHY(NULL)) )


/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */
EXTERN FlatHierarchy_ptr FlatHierarchy_create ARGS((SymbTable_ptr st));

EXTERN FlatHierarchy_ptr 
FlatHierarchy_create_from_members ARGS((SymbTable_ptr st,
                                        node_ptr init, 
                                        node_ptr invar, 
                                        node_ptr trans, 
                                        node_ptr input, 
                                        node_ptr justice, 
                                        node_ptr compassion));

EXTERN void  FlatHierarchy_destroy ARGS((FlatHierarchy_ptr self));

EXTERN FlatHierarchy_ptr 
FlatHierarchy_copy ARGS((const FlatHierarchy_ptr self));

EXTERN void FlatHierarchy_mergeinto ARGS((FlatHierarchy_ptr self,
                                          const FlatHierarchy_ptr other));

EXTERN SymbTable_ptr 
FlatHierarchy_get_symb_table ARGS((const FlatHierarchy_ptr self));

/* Access function to the class's fields : constrains and specifications */
EXTERN node_ptr FlatHierarchy_get_init ARGS((FlatHierarchy_ptr cmp));
EXTERN void FlatHierarchy_set_init ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_invar ARGS((FlatHierarchy_ptr cmp));
EXTERN void FlatHierarchy_set_invar ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_trans ARGS((FlatHierarchy_ptr cmp));
EXTERN void FlatHierarchy_set_trans ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_input ARGS((FlatHierarchy_ptr cmp));
EXTERN void FlatHierarchy_set_input ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_assign ARGS((FlatHierarchy_ptr cmp));
EXTERN void 
FlatHierarchy_set_assign ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_justice ARGS((FlatHierarchy_ptr cmp));
EXTERN void 
FlatHierarchy_set_justice ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_compassion ARGS((FlatHierarchy_ptr cmp));

EXTERN void FlatHierarchy_set_compassion ARGS((FlatHierarchy_ptr cmp, 
                                               node_ptr n));

EXTERN boolean FlatHierarchy_add_property_name ARGS((FlatHierarchy_ptr cmp, 
                                                  node_ptr name));

EXTERN node_ptr FlatHierarchy_get_spec ARGS((FlatHierarchy_ptr cmp));
EXTERN void FlatHierarchy_set_spec ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_ltlspec ARGS((FlatHierarchy_ptr cmp));
EXTERN void 
FlatHierarchy_set_ltlspec ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_invarspec ARGS((FlatHierarchy_ptr cmp));

EXTERN void FlatHierarchy_set_invarspec ARGS((FlatHierarchy_ptr cmp, 
                                              node_ptr n));

EXTERN node_ptr FlatHierarchy_get_pslspec ARGS((FlatHierarchy_ptr cmp));
EXTERN void 
FlatHierarchy_set_pslspec ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_compute ARGS((FlatHierarchy_ptr cmp));
EXTERN void 
FlatHierarchy_set_compute ARGS((FlatHierarchy_ptr cmp, node_ptr n));

/* -- access functions to the variable sets -- */
EXTERN Set_t FlatHierarchy_get_vars ARGS((FlatHierarchy_ptr cmp));
EXTERN void FlatHierarchy_add_var ARGS((FlatHierarchy_ptr cmp, node_ptr n));
EXTERN void FlatHierarchy_remove_var ARGS((FlatHierarchy_ptr self, node_ptr n));
EXTERN NodeList_ptr
FlatHierarchy_get_ordered_vars ARGS((const FlatHierarchy_ptr self,
                                     hash_ptr* outbound_edges));

EXTERN node_ptr FlatHierarchy_get_preds ARGS((FlatHierarchy_ptr cmp));
EXTERN void FlatHierarchy_add_pred ARGS((FlatHierarchy_ptr cmp, node_ptr n));
EXTERN void FlatHierarchy_set_pred ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN node_ptr FlatHierarchy_get_mirrors ARGS((FlatHierarchy_ptr cmp));
EXTERN void FlatHierarchy_add_mirror ARGS((FlatHierarchy_ptr cmp, node_ptr n));
EXTERN void FlatHierarchy_set_mirror ARGS((FlatHierarchy_ptr cmp, node_ptr n));

EXTERN void FlatHierarchy_self_check ARGS((const FlatHierarchy_ptr self));


/* Access function to the hash.
   Given a var name these functions can return right handside of 
   init-assignment, invar-assignmnent and next-assign which have the given
   variable on the left handside.
   The functions also can return the list of INIT, INVAR and TRANS expressions
   which contain the given variable.
*/

EXTERN node_ptr FlatHierarchy_lookup_assign ARGS((FlatHierarchy_ptr self,
                                                  node_ptr name));

EXTERN void FlatHierarchy_insert_assign ARGS((FlatHierarchy_ptr self,
                                              node_ptr name,
                                              node_ptr assign));

EXTERN node_ptr FlatHierarchy_lookup_constrains ARGS((FlatHierarchy_ptr self,
                                                      node_ptr name));

EXTERN void FlatHierarchy_add_constrains ARGS((FlatHierarchy_ptr self,
                                               node_ptr name,
                                               node_ptr expr));

EXTERN node_ptr
FlatHierarchy_lookup_constant_constrains ARGS((FlatHierarchy_ptr self,
                                               int type));

EXTERN void FlatHierarchy_add_constant_constrains ARGS((FlatHierarchy_ptr self,
                                                        node_ptr expr,
                                                        int type));
EXTERN void
FlatHierarchy_calculate_vars_constrains ARGS((FlatHierarchy_ptr self));

EXTERN hash_ptr
FlatHierarchy_get_var_expr_associations ARGS((FlatHierarchy_ptr self));
EXTERN void
FlatHierarchy_set_var_expr_associations ARGS((FlatHierarchy_ptr self,
                                              hash_ptr h));

EXTERN void
FlatHierarchy_clear_var_expr_associations ARGS((FlatHierarchy_ptr self));

EXTERN hash_ptr
FlatHierarchy_get_constants_associations ARGS((FlatHierarchy_ptr self));
EXTERN void
FlatHierarchy_set_constants_associations ARGS((FlatHierarchy_ptr self,
                                               hash_ptr h));
EXTERN void
FlatHierarchy_clear_constants_associations ARGS((FlatHierarchy_ptr self));

EXTERN void
FlatHierarchy_set_symb_table ARGS((const FlatHierarchy_ptr self,
                                   SymbTable_ptr symb_table));

#endif /* __FLAT_HIERARCHY_H__ */
