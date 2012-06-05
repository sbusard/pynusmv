/**CHeaderFile*****************************************************************

   FileName    [compile.h]

   PackageName [compile]

   Synopsis    [Compilation of NuSMV input language into BDD.]

   Description [This package contains the compiler of NuSMV code into
   BDD. It works on a flattened/instantiated structure. Performs the
   checks on the parse tree and fills a layer (when requested) and
   the symbol table. ]

   Author      [Marco Roveri, Emanuele Olivetti]

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

   Revision    [$Id: compile.h,v 1.44.4.17.2.1.2.25.4.50 2010-02-24 18:34:52 nusmv Exp $]

******************************************************************************/
#ifndef __COMPILE_H__
#define __COMPILE_H__

#include "FlatHierarchy.h"
#include "PredicateNormaliser.h"
#include "PredicateExtractor.h"

#include "symb_table/SymbTable.h"
#include "symb_table/SymbLayer.h"

#include "enc/bdd/BddEnc.h"

#include "utils/assoc.h"
#include "utils/utils.h"
#include "utils/NodeList.h"

#include "node/node.h"
#include "dd/dd.h"
#include "set/set.h"
#include "fsm/FsmBuilder.h"
#include "fsm/sexp/SexpFsm.h"
#include "fsm/sexp/BoolSexpFsm.h"
#include "fsm/sexp/Expr.h"
#include "hrc/HrcNode.h"


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct cmp_struct* cmp_struct_ptr;

typedef enum {
  State_Variables_Instantiation_Mode,
  Frozen_Variables_Instantiation_Mode,
  Input_Variables_Instantiation_Mode
} Instantiation_Variables_Mode_Type;

/**Enum************************************************************************

   Synopsis    [Enumerates the different types of a specification]

   Description [Enumerates the different types of a specification]

******************************************************************************/
typedef enum {ST_Notype, ST_Ctl, ST_Ltl, ST_Invar, ST_Compute} Spec_Type;


/**Enum************************************************************************

   Synopsis    [Enumerates the status of a specification]

   Description [Enumerates the status of a specification]

******************************************************************************/
typedef enum {SS_Nostatus, SS_Unchecked, SS_True, SS_False, SS_Wrong, SS_Number} Spec_Status;


typedef struct _Fsm_SexpRec    Fsm_SexpRec;
typedef struct _Fsm_SexpRec  * Fsm_SexpPtr;




#include "be/be.h" /* the generic boolean expressions interface */

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

   Synopsis     [The name of the standard layer dedicated to model symbols]

   Description  [Use this name when creating the layer of model symbols]

   SideEffects  []

******************************************************************************/
#define MODEL_LAYER_NAME  "model"


/**Macro***********************************************************************

   Synopsis     [The name of the standard layer dedicated to determinization
   variables introduced by the booleanization of the model]

   Description  [Use this name when creating the layer of determinization vars]

   SideEffects  []

******************************************************************************/
#define DETERM_LAYER_NAME "determ"

/**Macro***********************************************************************

   Synopsis     [The name of the standard layer dedicated to inlining symbols
   introduced by the inlining process of the model]

   Description  [Use this name when creating the layer of inlining symbols]

   SideEffects  []

******************************************************************************/
#define INLINING_LAYER_NAME "inlining"


/**Macro***********************************************************************

   Synopsis     [The symbolic name of the input process selector variable.]

   Description  [This is the internal symbolic name of process selector
   variable. The range of this variable is the set of names of the
   instantiated processes.]

   SideEffects  []

******************************************************************************/
#define PROCESS_SELECTOR_VAR_NAME "_process_selector_"

/**Macro***********************************************************************

   Synopsis     [The "running" symbol.]

   Description  [The "running" symbol used to refer the internal
   variable "running" of  processes.]

   SideEffects  []

   SeeAlso      []

******************************************************************************/
#define RUNNING_SYMBOL "running"


/**Macro***********************************************************************

   Synopsis     [The name of the model layers class, set to be the default]

   Description  []

   SideEffects  []

   SeeAlso      []

******************************************************************************/
#define MODEL_LAYERS_CLASS "Model Class"

/**Macro***********************************************************************

   Synopsis     [The name of the artefacts layers class, set to be the default]

   Description  []

   SideEffects  []

   SeeAlso      []

******************************************************************************/
#define ARTIFACTS_LAYERS_CLASS "Artifacts Class"


/*---------------------------------------------------------------------------*/
/* Global Variables                                                          */
/*---------------------------------------------------------------------------*/

EXTERN FlatHierarchy_ptr mainFlatHierarchy;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void Compile_init ARGS((void));
EXTERN void Compile_init_cmd ARGS((void));
EXTERN void Compile_quit  ARGS((void));

EXTERN void CompileFlatten_init_flattener ARGS((void));
EXTERN void CompileFlatten_quit_flattener ARGS((void));

EXTERN void CompileFlatten_hash_module ARGS((node_ptr parsed_module));

EXTERN FlatHierarchy_ptr
Compile_FlattenHierarchy ARGS((const SymbTable_ptr symb_table,
                               SymbLayer_ptr layer,
                               node_ptr, node_ptr, node_ptr,
                               boolean create_process_variable,
                               boolean calc_vars_constr,
                               HrcNode_ptr hrc_result));

EXTERN void Compile_CheckAssigns ARGS((const SymbTable_ptr, node_ptr));

EXTERN void Compile_check_case ARGS((node_ptr expr));

EXTERN void Compile_check_next ARGS((const SymbTable_ptr st,
                                     node_ptr expr, node_ptr context,
                                     boolean is_one_next_allowed));

EXTERN void Compile_check_input_next ARGS((const SymbTable_ptr st,
                                           node_ptr expr, node_ptr context));

EXTERN node_ptr
CompileFlatten_concat_contexts ARGS((node_ptr ctx1, node_ptr ctx2));

EXTERN node_ptr
CompileFlatten_expand_range ARGS((int a, int b));

EXTERN node_ptr
CompileFlatten_resolve_number ARGS((SymbTable_ptr symb_table,
                                    node_ptr n, node_ptr context));

EXTERN node_ptr
CompileFlatten_resolve_define_chains ARGS((const SymbTable_ptr symb_table,
                                           node_ptr expr, node_ptr context));
EXTERN node_ptr sym_intern ARGS((char *));


EXTERN void Compile_BuildVarsBdd ARGS((void));

EXTERN void build_proc_selector ARGS((node_ptr));
EXTERN void Compile_CompileInit ARGS((node_ptr, node_ptr));
EXTERN void Compile_CompileModel ARGS((node_ptr, node_ptr, node_ptr, add_ptr));

EXTERN void insert_module_hash ARGS((node_ptr x, node_ptr y));
EXTERN node_ptr lookup_module_hash ARGS((node_ptr x));
EXTERN node_ptr lookup_param_hash ARGS((node_ptr));

EXTERN void compileCheckForInputVars ARGS((SymbTable_ptr, node_ptr, node_ptr,
                                           node_ptr, node_ptr,
                                           FlatHierarchy_ptr hierarchy));


EXTERN int cmp_struct_get_read_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_read_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_unset_read_model ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_hrc_built ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_hrc_built ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_flatten_hrc ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_flatten_hrc ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_encode_variables ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_encode_variables ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_process_selector ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_process_selector ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_frames ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_frames ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_model ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_flat_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_flat_model ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_build_bool_model ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_build_bool_model ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_fairness ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_fairness ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_coi ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_coi ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_bmc_init ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_bmc_init ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_unset_bmc_init ARGS((cmp_struct_ptr cmp));
EXTERN int cmp_struct_get_bmc_setup ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_set_bmc_setup ARGS((cmp_struct_ptr cmp));
EXTERN void cmp_struct_unset_bmc_setup ARGS((cmp_struct_ptr cmp));

EXTERN node_ptr
Compile_FlattenSexp ARGS((const SymbTable_ptr symb_table, node_ptr, node_ptr));
EXTERN node_ptr
Compile_FlattenSexpExpandDefine ARGS((const SymbTable_ptr symb_table,
                                      node_ptr, node_ptr));


EXTERN void
Compile_WriteFlattenModel ARGS((FILE* out,
                                const SymbTable_ptr st,
                                const array_t* layer_names,
                                const char* fsm_name,
                                FlatHierarchy_ptr hierarchy,
                                boolean force_flattening));

EXTERN void
Compile_WriteRestrictedFlattenModel ARGS((FILE* out,
                                          const SymbTable_ptr st,
                                          const array_t* layer_names,
                                          const char* fsm_name,
                                          FlatHierarchy_ptr hierarchy,
                                          boolean force_flattening));

EXTERN void
Compile_WriteObfuscatedFlattenModel ARGS((FILE* out,
                                          const SymbTable_ptr st,
                                          const array_t* layer_names,
                                          const char* fsm_name,
                                          FlatHierarchy_ptr hierarchy,
                                          boolean print_map,
                                          boolean force_flattening));

EXTERN void
Compile_WriteFlattenFsm ARGS((FILE* out,
                              const SymbTable_ptr symb_table,
                              const array_t* layer_names,
                              const char* fsm_name,
                              FlatHierarchy_ptr hierarchy,
                              boolean force_flattening));

EXTERN void
Compile_WriteFlattenModel_udg ARGS((FILE* out,
                                    const SymbTable_ptr st,
                                    const array_t* layer_names,
                                    const char* fsm_name,
                                    FlatHierarchy_ptr hierarchy));
EXTERN void
Compile_WriteFlattenSpecs ARGS((FILE* out,
                                const SymbTable_ptr st,
                                FlatHierarchy_ptr hierarchy,
                                boolean force_flattening));

EXTERN void
Compile_WriteBoolModel ARGS((FILE* out,
                             BddEnc_ptr enc,
                             NodeList_ptr layers,
                             const char* fsm_name,
                             BoolSexpFsm_ptr bool_sexp_fsm,
                             boolean force_flattening));
EXTERN void
Compile_WriteBoolFsm ARGS((FILE* out,
                           const SymbTable_ptr symb_table,
                           NodeList_ptr layers,
                           const char* fsm_name,
                           BoolSexpFsm_ptr bool_sexp_fsm,
                           boolean force_flattening));

EXTERN void
Compile_WriteBoolSpecs ARGS((FILE* out,
                             BddEnc_ptr enc,
                             FlatHierarchy_ptr hierarchy));

EXTERN boolean Compile_is_expr_booleanizable ARGS((const SymbTable_ptr st,
                                                   node_ptr expr,
                                                   boolean word_booleanizable,
                                                   hash_ptr cache));

EXTERN Expr_ptr Compile_expr2bexpr ARGS((BddEnc_ptr enc,
                                         SymbLayer_ptr det_layer,
                                         Expr_ptr expr));

EXTERN Expr_ptr Compile_detexpr2bexpr ARGS((BddEnc_ptr enc, Expr_ptr expr));
EXTERN Expr_ptr
Compile_detexpr2bexpr_list ARGS((BddEnc_ptr enc, Expr_ptr expr));

EXTERN void Compile_cleanup_booleanizer_cache_about
ARGS((SymbTable_ptr st, NodeList_ptr symbs));

EXTERN Set_t
Formula_GetDependencies ARGS((const SymbTable_ptr, node_ptr, node_ptr));

EXTERN Set_t
Formula_GetDependenciesByType ARGS((const SymbTable_ptr, node_ptr, node_ptr,
                                    SymbFilterType, boolean));

EXTERN Set_t
Formula_GetConstants ARGS((const SymbTable_ptr symb_table,
                           node_ptr formula, node_ptr context));

EXTERN Set_t
Formulae_GetDependencies ARGS((const SymbTable_ptr, node_ptr, node_ptr,
                               node_ptr));

EXTERN Set_t ComputeCOIFixpoint ARGS((const SymbTable_ptr symb_table,
                                      const FlatHierarchy_ptr hierarchy,
                                      const Expr_ptr expression,
                                      const int steps,
                                      boolean* reached_fixpoint));

EXTERN Set_t ComputeCOI ARGS((const SymbTable_ptr, Set_t));

EXTERN void print_conjunctive_partition_info ARGS((FILE *, node_ptr));
EXTERN bdd_ptr Compile_BuildInitBdd ARGS((node_ptr));
EXTERN bdd_ptr Compile_BuildInvarBdd ARGS((node_ptr, add_ptr));

EXTERN void start_test ARGS((char *));
EXTERN void end_test ARGS((char *));

EXTERN void set_definition_mode_to_get ARGS((void));
EXTERN void set_definition_mode_to_expand ARGS((void));
EXTERN int definition_mode_is_expand ARGS((void));

EXTERN node_ptr
Flatten_GetDefinition ARGS((const SymbTable_ptr symb_table, node_ptr atom));

EXTERN void Flatten_remove_symbol_info ARGS((node_ptr name));

EXTERN SymbTable_ptr Compile_get_global_symb_table ARGS((void));

EXTERN PredicateNormaliser_ptr
Compile_get_global_predicate_normaliser ARGS((void));

EXTERN node_ptr
CompileFlatten_normalise_value_list ARGS((node_ptr old_value_list));

EXTERN FsmBuilder_ptr Compile_get_global_fsm_builder ARGS((void));

EXTERN node_ptr Compile_pop_distrib_ops ARGS((node_ptr prop));

EXTERN Set_t
Compile_make_sorted_vars_list_from_order
ARGS((const SymbTable_ptr st,
      const NodeList_ptr vars, const NodeList_ptr vars_order));

EXTERN int Compile_check_if_flattening_was_built ARGS((FILE* err));

EXTERN int Compile_check_if_encoding_was_built ARGS((FILE* err));

EXTERN int Compile_check_if_flat_model_was_built ARGS((FILE* err,
                                                       boolean forced));

EXTERN int Compile_check_if_bool_model_was_built ARGS((FILE* err,
                                                       boolean forced));

EXTERN int Compile_check_if_model_was_built ARGS((FILE* err,
                                                  boolean forced));

EXTERN SymbType_ptr
Compile_InstantiateType ARGS((SymbTable_ptr st, SymbLayer_ptr layer,
                               node_ptr name, node_ptr type,
                               node_ptr context));
EXTERN boolean
Compile_DeclareVariable ARGS((SymbTable_ptr symb_table, SymbLayer_ptr layer,
                              node_ptr name, SymbType_ptr type,
                              node_ptr context,
                              Instantiation_Variables_Mode_Type mode));

EXTERN node_ptr Compile_make_dag_info ARGS((node_ptr expr, hash_ptr hash));

EXTERN node_ptr Compile_convert_to_dag ARGS((SymbTable_ptr symb_table,
                                             node_ptr expr,
                                             hash_ptr hash,
                                             hash_ptr defines));

EXTERN void Compile_write_dag_defines ARGS((FILE* out, hash_ptr defines));

EXTERN void Compile_destroy_dag_info ARGS((hash_ptr dag_info, hash_ptr defines));

EXTERN void Compile_print_array_define ARGS((FILE* out, const node_ptr n));

EXTERN hash_ptr
Compile_get_obfuscation_map ARGS((const SymbTable_ptr symb_table));

EXTERN node_ptr
Compile_obfuscate_expression ARGS((const SymbTable_ptr symb_table,
                                   node_ptr expr,
                                   const hash_ptr obfuscation_map));

#endif /* __COMPILE_H__ */
