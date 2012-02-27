/**CHeaderFile*****************************************************************

  FileName    [BddEncCache.h]

  PackageName [enc.bdd]

  Synopsis    [The Bdd encoding cache interface]

  Description [This interface and relative class is intended to be
  used exclusively by the BddEnc class]
                                               
  SeeAlso     [BddEncCache.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bdd'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst. 

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

#ifndef __BDD_ENC_CACHE_H__
#define __BDD_ENC_CACHE_H__

#include "compile/symb_table/SymbTable.h"
#include "dd/dd.h"

#include "utils/utils.h"
#include "node/node.h"
#include "enc/utils/AddArray.h"

/**Type***********************************************************************

  Synopsis     [The BddEncCache type ]

  Description  [The BddEncCache type ]  

  Notes        []

******************************************************************************/
typedef struct BddEncCache_TAG*  BddEncCache_ptr;
#define BDD_ENC_CACHE(x) \
          ((BddEncCache_ptr) x)

#define BDD_ENC_CACHE_CHECK_INSTANCE(x) \
          ( nusmv_assert(BDD_ENC_CACHE(x) != BDD_ENC_CACHE(NULL)) )


/* ---------------------------------------------------------------------- */
/* Types                                                                  */
/* ---------------------------------------------------------------------- */

EXTERN node_ptr zero_number; /* constant from DD package */
EXTERN node_ptr one_number; /* constant from DD package */
/* ---------------------------------------------------------------------- */
/* Public methods                                                         */
/* ---------------------------------------------------------------------- */

EXTERN BddEncCache_ptr 
BddEncCache_create ARGS((SymbTable_ptr symb_table, DdManager* dd));

EXTERN void BddEncCache_destroy ARGS((BddEncCache_ptr self));

EXTERN void 
BddEncCache_new_constant ARGS((BddEncCache_ptr self, node_ptr constant, 
                               add_ptr constant_add));
EXTERN void 
BddEncCache_remove_constant ARGS((BddEncCache_ptr self, node_ptr constant));

EXTERN boolean 
BddEncCache_is_constant_encoded ARGS((const BddEncCache_ptr self, 
                                      node_ptr constant));
EXTERN add_ptr 
BddEncCache_lookup_constant ARGS((const BddEncCache_ptr self, 
                                  node_ptr constant));

EXTERN void 
BddEncCache_new_boolean_var ARGS((BddEncCache_ptr self, node_ptr var_name,
                                  add_ptr var_add));
EXTERN void 
BddEncCache_remove_boolean_var ARGS((BddEncCache_ptr self, node_ptr var_name));

EXTERN boolean 
BddEncCache_is_boolean_var_encoded ARGS((const BddEncCache_ptr self, 
                                         node_ptr var_name));
EXTERN add_ptr 
BddEncCache_lookup_boolean_var ARGS((const BddEncCache_ptr self, node_ptr var_name));

EXTERN void BddEncCache_set_evaluation ARGS((BddEncCache_ptr self,
                                             node_ptr expr,
                                             AddArray_ptr add_array));
EXTERN void BddEncCache_remove_evaluation ARGS((BddEncCache_ptr self,
                                                node_ptr expr));
EXTERN AddArray_ptr BddEncCache_get_evaluation ARGS((BddEncCache_ptr self,
                                                     node_ptr expr));

EXTERN void BddEncCache_clean_evaluation_about ARGS((BddEncCache_ptr self, 
                                                     NodeList_ptr symbs));
EXTERN void BddEncCache_clean_evaluation ARGS((BddEncCache_ptr self));

#endif /* __BDD_ENC_CACHE_H__ */
