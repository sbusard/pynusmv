/**CHeaderFile*****************************************************************

  FileName    [SymbCache_private.h]

  PackageName [compile.symb_table]

  Synopsis    [The SymbCache class private interface]

  Description []

  SeeAlso     [SymbCache.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.symb_table'' package of NuSMV
  version 2.  Copyright (C) 2004 by FBK-irst.

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

#ifndef __SYMB_CACHE_PRIVATE_H__
#define __SYMB_CACHE_PRIVATE_H__


#include "SymbCache.h"
#include "SymbTable.h"

#include "utils/utils.h"
#include "compile/symb_table/NFunction.h"
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Type definitions                                                          */
/*---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------- */
/*     Private methods                                                    */
/* ---------------------------------------------------------------------- */

EXTERN SymbCache_ptr SymbCache_create ARGS((SymbTable_ptr symb_table));

EXTERN void SymbCache_destroy ARGS((SymbCache_ptr self));

EXTERN void
SymbCache_new_input_var ARGS((SymbCache_ptr self,
                              node_ptr var, SymbType_ptr type));
EXTERN void
SymbCache_new_state_var ARGS((SymbCache_ptr self,
                              node_ptr var, SymbType_ptr type));
EXTERN void
SymbCache_new_frozen_var ARGS((SymbCache_ptr self,
                                node_ptr var, SymbType_ptr type));
EXTERN void
SymbCache_redeclare_state_as_frozen_var ARGS((SymbCache_ptr self,
                                              node_ptr var));

EXTERN void
SymbCache_remove_var ARGS((SymbCache_ptr self, node_ptr var));

EXTERN void
SymbCache_new_define ARGS((SymbCache_ptr self,
                           node_ptr name,
                           node_ptr context, node_ptr definition));
EXTERN void
SymbCache_new_function ARGS((SymbCache_ptr self, node_ptr name,
                               node_ptr context, NFunction_ptr fun));

EXTERN void
SymbCache_new_parameter ARGS((SymbCache_ptr self,
                              node_ptr formal,
                              node_ptr context, node_ptr actual));

EXTERN void
SymbCache_new_array_define ARGS((SymbCache_ptr self, node_ptr name,
                                  node_ptr ctx, node_ptr definition));

EXTERN void
SymbCache_new_variable_array ARGS((SymbCache_ptr self, node_ptr name,
                                   SymbType_ptr type));

EXTERN void
SymbCache_remove_define ARGS((SymbCache_ptr self, node_ptr define));

EXTERN void
SymbCache_remove_function ARGS((SymbCache_ptr self, node_ptr name));

EXTERN void
SymbCache_new_constant ARGS((SymbCache_ptr self, node_ptr name));

EXTERN void
SymbCache_remove_constant ARGS((SymbCache_ptr self, node_ptr constant));

EXTERN void
SymbCache_remove_symbols ARGS((SymbCache_ptr self,
                               const node_ptr* symbols,
                               const unsigned int size));

EXTERN SymbTableType
SymbCache_get_symbol_type ARGS((const SymbCache_ptr self,
                                const node_ptr symbol));

EXTERN void
SymbCache_gen_iter ARGS((const SymbCache_ptr self,
                         SymbTableIter* iter,
                         const unsigned int mask));

EXTERN void
SymbCache_next_iter ARGS((const SymbCache_ptr self,
                          SymbTableIter* iter));

EXTERN boolean SymbCache_is_iter_end ARGS((const SymbCache_ptr self,
                                           const SymbTableIter* iter));

EXTERN node_ptr SymbCache_iter_get_symbol ARGS((const SymbCache_ptr self,
                                                const SymbTableIter* iter));

EXTERN void SymbCache_iter_set_filter ARGS((const SymbCache_ptr self,
                                            SymbTableIter* iter,
                                            SymbTableIterFilterFun filter,
                                            void* arg));

EXTERN int SymbCache_get_constants_num ARGS((const SymbCache_ptr self));
EXTERN int SymbCache_get_state_vars_num ARGS((const SymbCache_ptr self));
EXTERN int SymbCache_get_frozen_vars_num ARGS((const SymbCache_ptr self));
EXTERN int SymbCache_get_input_vars_num ARGS((const SymbCache_ptr self));
EXTERN int SymbCache_get_defines_num ARGS((const SymbCache_ptr self));
EXTERN int SymbCache_get_functions_num ARGS((const SymbCache_ptr self));
EXTERN int SymbCache_get_parameters_num ARGS((const SymbCache_ptr self));
EXTERN int SymbCache_get_array_defines_num ARGS((const SymbCache_ptr self));
EXTERN int SymbCache_get_variable_arrays_num ARGS((const SymbCache_ptr self));


EXTERN void
SymbCache_add_trigger ARGS((const SymbCache_ptr self,
                            SymbTableTriggerFun trigger,
                            SymbTableTriggerAction action,
                            void* arg));

EXTERN void
SymbCache_remove_trigger ARGS((const SymbCache_ptr self,
                               SymbTableTriggerFun trigger,
                               SymbTableTriggerAction action));


#endif /* __SYMB_CACHE_PRIVATE_H__ */
