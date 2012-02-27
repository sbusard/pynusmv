/**CHeaderFile*****************************************************************

  FileName    [SymbTable.h]

  PackageName [compile.symb_table]

  Synopsis    [The system wide symbol table interface]

  Description []

  SeeAlso     [SymbTable.c]

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

#ifndef __SYMB_TABLE_H__
#define __SYMB_TABLE_H__

#include "SymbLayer.h"
#include "SymbCache.h"

#include "compile/type_checking/TypeChecker.h"


#include "utils/utils.h"
#include "utils/array.h"
#include "utils/assoc.h"
#include "node/node.h"
#include "compile/symb_table/NFunction.h"
#include "set/set.h"
#include "ResolveSymbol.h"

/**Type***********************************************************************

  Synopsis    [SymbTable class accessors]

  Description []

******************************************************************************/
typedef struct SymbTable_TAG* SymbTable_ptr;

#define SYMB_TABLE(x)  \
        ((SymbTable_ptr) x)

#define SYMB_TABLE_CHECK_INSTANCE(x)  \
        (nusmv_assert(SYMB_TABLE(x) != SYMB_TABLE(NULL)))


/**Enum************************************************************************

  Synopsis    [Controls the filter type in some search dependencies routines]

  Description [Controls the filter type in some search dependencies routines.
  The available filters are:
  <dl>
    <dt> <tt>VFT_CURRENT</tt>
       <dd> filters out the current state variables</dd>
    </dt>
    <dt> <tt>VFT_NEXT</tt>
       <dd> filters out the next state variables</dd>
    </dt>
    <dt> <tt>VFT_INPUT</tt>
       <dd> filters out the input variables</dd>
    </dt>
    <dt> <tt>VFT_FROZEN</tt>
       <dd> filters out the frozen variables</dd>
    </dt>
    <dt> <tt>VFT_STATE</tt>
       <dd> filters out the current and next state variables</dd>
    </dt>
    <dt> <tt>VFT_CURR_INPUT</tt>
       <dd> filters out the current state and input variables</dd>
    </dt>
    <dt> <tt>VFT_CURR_FROZEN</tt>
       <dd> filters out the current state and frozen variables</dd>
    </dt>
    <dt> <tt>VFT_ALL</tt>
       <dd>filters out all the variables</dd>
    </dt>
  </dl>
  Combined modes can be obtained by bit-or: for example
  VFT_NEXT | VFT_INPUT is going to search for the variables which
  are next or input variables]

******************************************************************************/
typedef enum SymbFilterType_TAG {VFT_CURRENT=1, VFT_NEXT=2, VFT_STATE=3,
                                 VFT_INPUT=4, VFT_CURR_INPUT=5,
                                 VFT_FROZEN=8, VFT_CURR_FROZEN=9,
                                 VFT_ALL=15, VFT_DEFINE=16,
                                 VFT_ALL_DEFINE=31} SymbFilterType;


/**Enum************************************************************************

  Synopsis    [Describes the kind of symbol]

  Description [Symbols categorization is used to have a fine grained
               control on the various types of symbols (defines, vars)
               that be encountered in an expression. While for
               variables categorization is straightforward, when it
               comes to defines the categorization depends on the
               categories of the variables that appear in the expanded
               body of the define. The categories are a partition of
               the entire set of symbols. (i.e. categories are
               exclusive)

               In particular:

               0. CONSTANT defines do not depend on any var.
               1. STATE defines depend on current state vars.
               2. INPUT defines depend on input vars.
               3. STATE-INPUT defines depend on both current state and input vars.
               4. NEXT defines depend on next state vars.
               5. STATE-NEXT defines depend on both current and next state vars.
               6. INPUT-NEXT defines depend on both input vars and next state vars.
               7. STATE-INPUT-NEXT defines depend on current state vars, input vars,
                  next state vars.

               Remark: w.r.t. DEFINE categorization, frozen vars are
               considered as state vars.

******************************************************************************/
typedef enum SymbCategory_TAG {
  SYMBOL_INVALID = 0, /* This is required by current implementation */
  SYMBOL_CONSTANT,
  SYMBOL_FROZEN_VAR,
  SYMBOL_STATE_VAR,
  SYMBOL_INPUT_VAR,
  SYMBOL_STATE_DEFINE,
  SYMBOL_INPUT_DEFINE,
  SYMBOL_STATE_INPUT_DEFINE,
  SYMBOL_NEXT_DEFINE,
  SYMBOL_STATE_NEXT_DEFINE,
  SYMBOL_INPUT_NEXT_DEFINE,
  SYMBOL_STATE_INPUT_NEXT_DEFINE,
  SYMBOL_DEFINE,
  SYMBOL_FUNCTION,
  SYMBOL_PARAMETER,
  SYMBOL_ARRAY_DEFINE,
  SYMBOL_VARIABLE_ARRAY
} SymbCategory;

typedef enum SymbTableType_TAG {
  STT_NONE           = 0,
  STT_CONSTANT       = 1,

  STT_STATE_VAR      = STT_CONSTANT << 1, /* 2 */
  STT_INPUT_VAR      = STT_CONSTANT << 2, /* 4 */
  STT_FROZEN_VAR     = STT_CONSTANT << 3, /* 8 */
  STT_VAR            = STT_STATE_VAR | STT_INPUT_VAR | STT_FROZEN_VAR, /* 14 */

  STT_DEFINE         = STT_CONSTANT << 4, /* 16 */
  STT_ARRAY_DEFINE   = STT_CONSTANT << 5, /* 32 */

  STT_PARAMETER      = STT_CONSTANT << 6, /* 64 */

  STT_FUNCTION       = STT_CONSTANT << 7, /* 128 */

  STT_VARIABLE_ARRAY = STT_CONSTANT << 8, /* 256 */

  STT_ALL            = STT_CONSTANT | STT_VAR | STT_DEFINE | STT_ARRAY_DEFINE | \
                       STT_PARAMETER | STT_FUNCTION | STT_VARIABLE_ARRAY, /* 511 */
} SymbTableType;

typedef enum SymbTableTriggerAction_TAG {
  ST_TRIGGER_SYMBOL_ADD,
  ST_TRIGGER_SYMBOL_REMOVE,
  ST_TRIGGER_SYMBOL_REDECLARE
} SymbTableTriggerAction;

typedef void (*SymbTableForeachFun)(const SymbTable_ptr,
                                    const node_ptr sym,
                                    void* arg);

typedef boolean (*SymbTableIterFilterFun)(const SymbTable_ptr table,
                                          const node_ptr sym, void* arg);

typedef void (*SymbTableTriggerFun)(const SymbTable_ptr table,
                                    const node_ptr sym,
                                    void* arg);

typedef struct SymbTableIter_TAG {
  unsigned int index;
  unsigned int mask;
  SymbTableIterFilterFun filter;
  SymbTable_ptr st;
  void* arg;
} SymbTableIter;

#define SYMB_TABLE_FOREACH(self, iter, mask)            \
  for (SymbTable_gen_iter(self, &iter, mask);           \
       !SymbTable_iter_is_end(self, &iter);             \
       SymbTable_iter_next(self, &iter))

#define SYMB_TABLE_FOREACH_FILTER(self, iter, mask, filter, arg)        \
  for (SymbTable_gen_iter(self, &iter, mask),                           \
         SymbTable_iter_set_filter(self, &iter, filter, arg);           \
       !SymbTable_iter_is_end(self, &iter);                             \
       SymbTable_iter_next(self, &iter))

/* ---------------------------------------------------------------------- */
/*     Public methods                                                     */
/* ---------------------------------------------------------------------- */

EXTERN SymbTable_ptr SymbTable_create ARGS((void));

EXTERN void SymbTable_destroy ARGS((SymbTable_ptr self));

EXTERN TypeChecker_ptr
SymbTable_get_type_checker ARGS((const SymbTable_ptr self));

/* -------------------------------------- */
/*            ITERATORS                   */
/* -------------------------------------- */
EXTERN void SymbTable_gen_iter ARGS((const SymbTable_ptr self,
                                     SymbTableIter* iter,
                                     unsigned int mask));

EXTERN void SymbTable_iter_next ARGS((const SymbTable_ptr self,
                                      SymbTableIter* iter));

EXTERN boolean SymbTable_iter_is_end ARGS((const SymbTable_ptr self,
                                           const SymbTableIter* iter));

EXTERN node_ptr SymbTable_iter_get_symbol ARGS((const SymbTable_ptr self,
                                                const SymbTableIter* iter));

EXTERN void SymbTable_iter_set_filter ARGS((const SymbTable_ptr self,
                                            SymbTableIter* iter,
                                            SymbTableIterFilterFun fun,
                                            void* arg));

EXTERN void SymbTable_foreach ARGS((const SymbTable_ptr self, unsigned int mask,
                                    SymbTableForeachFun fun, void* arg));

EXTERN Set_t SymbTable_iter_to_set ARGS((const SymbTable_ptr self,
                                         SymbTableIter iter));

EXTERN NodeList_ptr SymbTable_iter_to_list ARGS((const SymbTable_ptr self,
                                                 SymbTableIter iter));

EXTERN unsigned int SymbTable_iter_count ARGS((const SymbTable_ptr self,
                                               SymbTableIter iter));

EXTERN void
SymbTable_add_trigger ARGS((const SymbTable_ptr self,
                            SymbTableTriggerFun trigger,
                            SymbTableTriggerAction action,
                            void* arg));

EXTERN void
SymbTable_remove_trigger ARGS((const SymbTable_ptr self,
                               SymbTableTriggerFun trigger,
                               SymbTableTriggerAction action));

/* -------------------------------------- */
/*            Built-in filters            */
/* -------------------------------------- */
EXTERN boolean
SymbTable_iter_filter_i_symbols ARGS((const SymbTable_ptr self,
                                      const node_ptr sym,
                                      void* arg));

EXTERN boolean
SymbTable_iter_filter_sf_i_symbols ARGS((const SymbTable_ptr self,
                                         const node_ptr sym,
                                         void* arg));

EXTERN boolean
SymbTable_iter_filter_sf_symbols ARGS((const SymbTable_ptr self,
                                       const node_ptr sym,
                                       void* arg));


/* -------------------------------------- */
/*            Layers handling             */
/* -------------------------------------- */
EXTERN SymbLayer_ptr
SymbTable_create_layer ARGS((SymbTable_ptr self, const char* layer_name,
                             const LayerInsertPolicy ins_policy));

EXTERN void
SymbTable_remove_layer ARGS((SymbTable_ptr self, SymbLayer_ptr layer));

EXTERN SymbLayer_ptr
SymbTable_get_layer ARGS((const SymbTable_ptr self,
                          const char* layer_name));

EXTERN void
SymbTable_rename_layer ARGS((const SymbTable_ptr self,
                             const char* layer_name, const char* new_name));

EXTERN NodeList_ptr SymbTable_get_layers ARGS((const SymbTable_ptr self));


/* -------------------------------------- */
/*                Symbols                 */
/* -------------------------------------- */

/* Lists of symbols: */

EXTERN NodeList_ptr
SymbTable_get_layers_sf_symbols ARGS((SymbTable_ptr self,
                                      const array_t* layer_names));
EXTERN NodeList_ptr
SymbTable_get_layers_sf_vars ARGS((SymbTable_ptr self,
                                   const array_t* layer_names));
EXTERN NodeList_ptr
SymbTable_get_layers_i_symbols ARGS((SymbTable_ptr self,
                                     const array_t* layer_names));
EXTERN NodeList_ptr
SymbTable_get_layers_i_vars ARGS((SymbTable_ptr self,
                                  const array_t* layer_names));
EXTERN NodeList_ptr
SymbTable_get_layers_sf_i_symbols ARGS((SymbTable_ptr self,
                                        const array_t* layer_names));

EXTERN NodeList_ptr
SymbTable_get_layers_sf_i_vars ARGS((SymbTable_ptr self,
                                     const array_t* layer_names));

/* Number of symbols: */
EXTERN int SymbTable_get_vars_num ARGS((const SymbTable_ptr self));

EXTERN int SymbTable_get_state_vars_num ARGS((const SymbTable_ptr self));

EXTERN int SymbTable_get_frozen_vars_num ARGS((const SymbTable_ptr self));

EXTERN int SymbTable_get_input_vars_num ARGS((const SymbTable_ptr self));

EXTERN int SymbTable_get_defines_num ARGS((const SymbTable_ptr self));

EXTERN int SymbTable_get_array_defines_num ARGS((const SymbTable_ptr self));

EXTERN int SymbTable_get_parameters_num ARGS((const SymbTable_ptr self));

EXTERN int SymbTable_get_constants_num ARGS((const SymbTable_ptr self));

EXTERN int SymbTable_get_functions_num ARGS((const SymbTable_ptr self));


/* Classes of layers: */
EXTERN void
SymbTable_create_layer_class ARGS((SymbTable_ptr self,
                                   const char* class_name));

EXTERN boolean
SymbTable_layer_class_exists ARGS((SymbTable_ptr self,
                                   const char* class_name));


EXTERN void
SymbTable_layer_add_to_class ARGS((SymbTable_ptr self,
                                   const char* layer_name,
                                   const char* class_name));

EXTERN void
SymbTable_layer_remove_from_class ARGS((SymbTable_ptr self,
                                        const char* layer_name,
                                        const char* class_name));

EXTERN array_t*
SymbTable_get_class_layer_names ARGS((SymbTable_ptr self,
                                      const char* class_name));

EXTERN boolean
SymbTable_is_layer_in_class ARGS((SymbTable_ptr self,
                                  const char* layer_name,
                                  const char* class_name));
EXTERN void
SymbTable_set_default_layers_class_name ARGS((SymbTable_ptr self,
                                              const char* class_name));

EXTERN const char*
SymbTable_get_default_layers_class_name ARGS((const SymbTable_ptr self));


/* Symbols related info: */
EXTERN SymbType_ptr
SymbTable_get_var_type ARGS((const SymbTable_ptr self, const node_ptr name));

EXTERN node_ptr
SymbTable_get_define_body ARGS((const SymbTable_ptr self,
                                const node_ptr name));
EXTERN NFunction_ptr
SymbTable_get_function ARGS((const SymbTable_ptr self,
                               const node_ptr name));
EXTERN node_ptr
SymbTable_get_actual_parameter ARGS((const SymbTable_ptr self,
                                     const node_ptr name));

EXTERN node_ptr
SymbTable_get_array_define_body ARGS((const SymbTable_ptr self,
                                       const node_ptr name));
EXTERN SymbType_ptr
SymbTable_get_variable_array_type ARGS((const SymbTable_ptr self,
                                       const node_ptr name));

EXTERN node_ptr
SymbTable_get_define_flatten_body ARGS((const SymbTable_ptr self,
                                        const node_ptr name));

EXTERN node_ptr
SymbTable_get_flatten_actual_parameter ARGS((const SymbTable_ptr self,
                                               const node_ptr name));

EXTERN node_ptr
SymbTable_get_array_define_flatten_body ARGS((const SymbTable_ptr self,
                                               const node_ptr name));

EXTERN node_ptr
SymbTable_get_define_context ARGS((const SymbTable_ptr self,
                                   const node_ptr name));

EXTERN node_ptr
SymbTable_get_function_context ARGS((const SymbTable_ptr self,
                                       const node_ptr name));

EXTERN node_ptr
SymbTable_get_actual_parameter_context ARGS((const SymbTable_ptr self,
                                   const node_ptr name));

EXTERN node_ptr
SymbTable_get_array_define_context ARGS((const SymbTable_ptr self,
                                          const node_ptr name));

EXTERN SymbCategory
SymbTable_get_symbol_category ARGS((const SymbTable_ptr self,
                                    const node_ptr name));

/* Queries: */
EXTERN boolean
SymbTable_is_symbol_state_var ARGS((const SymbTable_ptr self,
                                    const node_ptr name));
EXTERN boolean
SymbTable_is_symbol_frozen_var ARGS((const SymbTable_ptr self,
                                     const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_state_frozen_var ARGS((const SymbTable_ptr self,
                                           const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_input_var ARGS((const SymbTable_ptr self,
                                    const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_var ARGS((const SymbTable_ptr self, const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_bool_var ARGS((const SymbTable_ptr self,
                                   const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_declared ARGS((const SymbTable_ptr self,
                                   const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_define ARGS((const SymbTable_ptr self,
                                 const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_function ARGS((const SymbTable_ptr self,
                                     const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_parameter ARGS((const SymbTable_ptr self,
                                 const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_array_define ARGS((const SymbTable_ptr self,
                                        const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_variable_array ARGS((const SymbTable_ptr self,
                                      const node_ptr name));

EXTERN boolean
SymbTable_is_symbol_constant ARGS((const SymbTable_ptr self,
                                   const node_ptr name));

EXTERN boolean
SymbTable_is_var_finite ARGS((const SymbTable_ptr self, const node_ptr name));


EXTERN boolean
SymbTable_list_contains_input_var ARGS((const SymbTable_ptr self,
                                        const NodeList_ptr var_list));

EXTERN boolean
SymbTable_list_contains_state_frozen_var ARGS((const SymbTable_ptr self,
                                               const NodeList_ptr var_list));

EXTERN boolean
SymbTable_list_contains_undef_var ARGS((const SymbTable_ptr self,
                                        const NodeList_ptr var_list));

EXTERN boolean
SymbTable_contains_infinite_precision_variables ARGS((const SymbTable_ptr self));

EXTERN SymbLayer_ptr
SymbTable_variable_get_layer ARGS((SymbTable_ptr  self, node_ptr name));

EXTERN SymbLayer_ptr
SymbTable_define_get_layer ARGS((SymbTable_ptr  self, node_ptr name));

EXTERN SymbLayer_ptr
SymbTable_symbol_get_layer ARGS((SymbTable_ptr  self, node_ptr name));

EXTERN SymbLayer_ptr
SymbTable_function_get_layer ARGS((SymbTable_ptr  self, node_ptr name));

EXTERN node_ptr
SymbTable_get_determinization_var_name ARGS((const SymbTable_ptr self));

EXTERN node_ptr
SymbTable_get_fresh_symbol_name ARGS((SymbTable_ptr self,
                                      const char * tplate));

EXTERN hash_ptr
SymbTable_get_simplification_hash ARGS((SymbTable_ptr self));

EXTERN const char*
SymbTable_get_class_of_layer ARGS((const SymbTable_ptr self,
                                   const char* layer_name));

ResolveSymbol_ptr
SymbTable_resolve_symbol ARGS((SymbTable_ptr self,
                               node_ptr expr, node_ptr context));

EXTERN SymbTable_ptr SymbTable_copy ARGS((SymbTable_ptr self,
                                          Set_t blacklist));

#endif /* __SYMB_TABLE_H__ */
