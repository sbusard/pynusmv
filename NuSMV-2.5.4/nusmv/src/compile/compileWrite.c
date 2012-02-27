/**CFile***********************************************************************

   FileName    [compileWrite.c]

   PackageName [compile]

   Synopsis [Creation of an SMV file containing the flattened or booleanized
   model.]

   Description [Creation of an SMV file containing the flattened or
   booleanized model, processes will be removed by explicitly
   introducing a process variable and modifying assignments to take
   care of inertia.]

   SeeAlso     []

   Author      [Marco Roveri, Roberto Cavada]

   Copyright   [
   This file is part of the ``compile'' package of NuSMV version 2.
   Copyright (C) 2004 by FBK-irst.

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

#include "symb_table/SymbLayer.h"
#include "symb_table/ResolveSymbol.h"
#include "parser/symbols.h"
#include "parser/psl/pslNode.h"
#include "opt/opt.h"
#include "utils/assoc.h"
#include "utils/ustring.h"


/*---------------------------------------------------------------------------*/
static char rcsid[] UTIL_UNUSED = "$Id: compileWrite.c,v 1.1.2.24.4.64 2010-01-29 15:22:32 nusmv Exp $";
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* Return value in case an error occurs */
#define TYPE_ERROR ((node_ptr) -1)


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static unsigned int defines_count = 0;
static unsigned int dag_hits = 0;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define GET_DAG_HITS_NUMBER() dag_hits
#define RESET_DAG_HITS_NUMBER() dag_hits = 0
#define INCREMENT_HITS_NUMBER() dag_hits++

/* Write statistics if requested */
#define PRINT_DAG_STATS() if (opt_get_daggifier_statistics(OptsHandler_get_instance())) {  \
  int i;                                                                \
  for(i=0; i<80; i++) fprintf(nusmv_stderr, "*");                       \
  fprintf(nusmv_stderr, "\n DAG Statistics:\n");                        \
  fprintf(nusmv_stderr,                                                 \
          "\tNumber of introduced defines: %d\n",                       \
          st_count(defines));                                           \
  fprintf(nusmv_stderr,                                                 \
          "\tNumber of hits: %d\n",                                     \
          GET_DAG_HITS_NUMBER());                                       \
  for(i=0; i<80; i++) fprintf(nusmv_stderr, "*");                       \
  fprintf(nusmv_stderr, "\n");                                          \
  }

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static boolean is_array_define_element ARGS((const SymbTable_ptr st,
                                             const node_ptr name));

static boolean is_array_var_element ARGS((const SymbTable_ptr st,
                                          const node_ptr name));

/* many functions can be merged with a parameter to distinguish :
   normal output, obfuscated output and restricted output.
*/

/* -- functions related to defines ------- */
static int
compile_write_flat_define ARGS((const SymbTable_ptr symb_table, FILE* out,
                                const NodeList_ptr names, hash_ptr, hash_ptr,
                                boolean force_flattening));
static int
compile_write_obfuscated_flat_define ARGS((const SymbTable_ptr symb_table,
                                           FILE* out,
                                           const SymbLayer_ptr layer,
                                           hash_ptr, hash_ptr, hash_ptr,
                                           boolean force_flattening));
static int
compile_write_flat_define_aux ARGS((const SymbTable_ptr symbol_table,
                                    FILE* out,
                                    node_ptr name,
                                    hash_ptr dag_info,
                                    hash_ptr defines,
                                    hash_ptr printed_arrays,
                                    boolean force_flattening));
static int
compile_write_obfuscated_flat_define_aux ARGS((const SymbTable_ptr symb_table,
                                               FILE* out,
                                               node_ptr name,
                                               hash_ptr dag_info,
                                               hash_ptr defines,
                                               hash_ptr printed_arrays,
                                               hash_ptr obfuscation_map,
                                               boolean force_flattening));

/* -- functions related to vars ------- */
static int
compile_write_flatten_vars ARGS((const SymbTable_ptr symb_table,
                                 FILE* out, const SymbLayer_ptr layer,
                                 SymbLayerIter* iter));

static int
compile_write_obfuscated_flatten_vars ARGS((const SymbTable_ptr symb_table,
                                            FILE* out,
                                            const SymbLayer_ptr layer, 
                                            const SymbTableType type,
                                            hash_ptr obfuscation_map));

static int
compile_write_flatten_vars_aux ARGS((const SymbTable_ptr layer,
                                     const node_ptr cell,
                                     FILE* out,
                                     hash_ptr printed));
static boolean
compile_write_obfuscated_flatten_vars_aux ARGS((const SymbTable_ptr st,
                                                const node_ptr name,
                                                FILE* out,
                                                hash_ptr printed,
                                                hash_ptr obfuscation_map));

/* -- functions related to FSM ------- */



static void
compile_print_assign ARGS((SymbTable_ptr st,
                           FILE *, node_ptr, node_ptr, hash_ptr, hash_ptr));

static int
compile_write_flat_asgn ARGS((const SymbTable_ptr symb_table, FILE* out,
                              const NodeList_ptr names,
                              FlatHierarchy_ptr hierarchy,
                              hash_ptr, hash_ptr, hash_ptr));

static int
compile_write_obfuscated_flat_asgn ARGS((const SymbTable_ptr symb_table,
                                         FILE* out,
                                         const SymbLayer_ptr layer,
                                         FlatHierarchy_ptr hierarchy,
                                         hash_ptr, hash_ptr,
                                         hash_ptr obfuscation_map,
                                         hash_ptr cdh));

static int
compile_write_flatten_bool_vars ARGS((const SymbTable_ptr symb_table,
                                      const BoolEnc_ptr bool_enc,
                                      FILE* out, const SymbLayer_ptr layer, 
                                      const SymbTableType type));

static int
compile_write_flatten_expr ARGS((const SymbTable_ptr symb_table,
                                 FILE* out, node_ptr l, const char* s,
                                 hash_ptr, hash_ptr,
                                 boolean force_flattening, hash_ptr cdh));

static int
compile_write_obfuscated_flatten_expr ARGS((const SymbTable_ptr symb_table,
                                            FILE* out,
                                            node_ptr l,
                                            const char* s,
                                            hash_ptr,
                                            hash_ptr,
                                            hash_ptr defines,
                                            boolean force_flattening,
                                            hash_ptr cdh));

static int
compile_write_flatten_expr_split ARGS((const SymbTable_ptr symb_table,
                                       FILE* out, node_ptr n, const char* s,
                                       hash_ptr dag_info, hash_ptr defines,
                                       boolean force_flattening,
                                       hash_ptr cdh));

static int
compile_write_obfuscated_flatten_expr_split ARGS((const SymbTable_ptr symb_table,
                                                  FILE* out, node_ptr n,
                                                  const char* s,
                                                  hash_ptr dag_info,
                                                  hash_ptr defines,
                                                  hash_ptr obfuscation_map,
                                                  boolean force_flattening,
                                                  hash_ptr cdh));

static int
compile_write_flatten_spec ARGS((const SymbTable_ptr symb_table,
                                 FILE* out, node_ptr l, const char* s,
                                 hash_ptr, hash_ptr,
                                 boolean force_flattening, hash_ptr cdh));

static int
compile_write_obfuscated_flatten_spec ARGS((const SymbTable_ptr symb_table,
                                            FILE* out,
                                            node_ptr l,
                                            const char* s,
                                            hash_ptr, hash_ptr, hash_ptr,
                                            boolean force_flattening,
                                            hash_ptr cdh));

static int
compile_write_flatten_spec_split ARGS((const SymbTable_ptr symb_table,
                                       FILE* out, node_ptr n, const char* s,
                                       hash_ptr dag_info, hash_ptr defines,
                                       boolean force_flattening,
                                       hash_ptr cdh));

static int
compile_write_obfuscated_flatten_spec_split ARGS((const SymbTable_ptr symb_table,
                                                  FILE* out,
                                                  node_ptr n,
                                                  const char* s,
                                                  hash_ptr dag_info,
                                                  hash_ptr defines,
                                                  hash_ptr obfuscation_map,
                                                  boolean force_flattening,
                                                  hash_ptr cdh));

static int
compile_write_flatten_expr_pair ARGS((const SymbTable_ptr symb_table,
                                      FILE* out, node_ptr l, const char* s,
                                      hash_ptr, hash_ptr,
                                      boolean force_flattening,
                                      hash_ptr cdh));

static int
compile_write_obfuscated_flatten_expr_pair ARGS((const SymbTable_ptr symb_table,
                                                 FILE* out,
                                                 node_ptr l,
                                                 const char* s,
                                                 hash_ptr,
                                                 hash_ptr,
                                                 hash_ptr obfuscation_map,
                                                 boolean force_flattening,
                                                 hash_ptr cdh));

static int
compile_write_flatten_bfexpr ARGS((BddEnc_ptr enc,
                                   const SymbTable_ptr symb_table,
                                   SymbLayer_ptr det_layer,
                                   FILE* out, node_ptr n, const char* s,
                                   hash_ptr, hash_ptr, hash_ptr cdh));

static int
compile_write_flatten_psl ARGS((const SymbTable_ptr symb_table,
                                FILE* out, node_ptr n,
                                hash_ptr, hash_ptr, hash_ptr));

static void
compile_write_flat_fsm ARGS((FILE* out,
                             const SymbTable_ptr symb_table,
                             const array_t* layer_names,
                             const char* fsm_name,
                             FlatHierarchy_ptr hierarchy,
                             hash_ptr dag_info, hash_ptr defines,
                             boolean force_flattening,
                             hash_ptr cdh));

static boolean
compile_write_is_var_in_set ARGS((const SymbLayer_ptr layer,
                                  const node_ptr sym, void* arg));

static void
compile_write_restricted_flat_fsm ARGS((FILE* out,
                                        const SymbTable_ptr symb_table,
                                        const array_t* layer_names,
                                        const char* fsm_name,
                                        FlatHierarchy_ptr hierarchy,
                                        hash_ptr dag_info, hash_ptr defines,
                                        boolean force_flattening,
                                        hash_ptr cdh));

static void
compile_write_obfuscated_flat_fsm ARGS((FILE* out,
                                        const SymbTable_ptr symb_table,
                                        const array_t* layer_names,
                                        const char* fsm_name,
                                        FlatHierarchy_ptr hierarchy,
                                        hash_ptr dag_info,
                                        hash_ptr defines,
                                        hash_ptr obfuscation_map,
                                        boolean force_flattening,
                                        hash_ptr cdh));

static void
compile_write_bool_fsm ARGS((FILE* out,
                             const SymbTable_ptr symb_table,
                             NodeList_ptr layers,
                             const char* fsm_name,
                             BoolSexpFsm_ptr bool_sexp_fsm,
                             hash_ptr dag_info, hash_ptr defines,
                             boolean force_flattening, hash_ptr cdh));

static void
compile_write_flat_spec ARGS((FILE* out, const SymbTable_ptr symb_table,
                              node_ptr spec, const char* msg,
                              hash_ptr dag_info, hash_ptr defines,
                              boolean force_flattening,
                              hash_ptr cdh));

static void
compile_write_flat_specs ARGS((FILE* out,
                               const SymbTable_ptr st,
                               FlatHierarchy_ptr hierarchy,
                               hash_ptr dag_info, hash_ptr defines,
                               boolean force_flattening,
                               hash_ptr cdh));

static void
compile_write_obfuscated_flat_specs ARGS((FILE* out,
                                          const SymbTable_ptr st,
                                          FlatHierarchy_ptr hierarchy,
                                          hash_ptr dag_info, hash_ptr defines,
                                          hash_ptr obfuscation_map,
                                          boolean force_flattening,
                                          hash_ptr cdh));

static void
compile_write_bool_spec ARGS((FILE* out, BddEnc_ptr enc,
                              node_ptr spec, const char* msg,
                              SymbLayer_ptr det_layer,
                              hash_ptr dag_info, hash_ptr defines,
                              hash_ptr cdh));

static void
compile_write_bool_specs ARGS((FILE* out,
                               BddEnc_ptr enc,
                               SymbLayer_ptr det_layer,
                               FlatHierarchy_ptr hierarchy,
                               hash_ptr dag_info,
                               hash_ptr defines,
                               hash_ptr cdh));

static int
compile_write_constants ARGS((const SymbTable_ptr symb_table, FILE* out));

static int
compile_write_obfuscated_constants ARGS((const SymbTable_ptr symb_table,
                                         FILE* out, hash_ptr obfuscation_map));

static hash_ptr
compile_create_dag_info_from_hierarchy ARGS((SymbTable_ptr st,
                                             FlatHierarchy_ptr hierarchy,
                                             SymbLayer_ptr det_layer,
                                             BddEnc_ptr enc,
                                             boolean force_flattening,
                                             hash_ptr cdh));

static node_ptr compile_pack_dag_info ARGS((unsigned int count,
                                            unsigned int depth,
                                            boolean admissible));
static void compile_unpack_dag_info ARGS((node_ptr info,
                                          unsigned int* count,
                                          unsigned int* depth,
                                          boolean* admissible));

static void compile_set_dag_info ARGS((node_ptr info,
                                       unsigned int count,
                                       unsigned int depth,
                                       boolean admissible));

static node_ptr compile_convert_to_dag_aux ARGS((SymbTable_ptr symb_table,
                                                 node_ptr expr, hash_ptr hash,
                                                 unsigned int num_thres,
                                                 unsigned int dep_thres,
                                                 hash_ptr defines,
                                                 const char* defines_prefix));

static node_ptr compile_make_dag_info_aux ARGS((node_ptr expr, hash_ptr hash));

static assoc_retval compile_free_node ARGS((char *key, char *data, char * arg));

static assoc_retval compile_free_define ARGS((char *key, char *data, char * arg));

static void
compile_symbtype_obfuscated_print ARGS((SymbType_ptr type,
                                        FILE* out,
                                        const SymbTable_ptr symb_table,
                                        hash_ptr obfuscation_map));

static void
compile_write_obfuscated_dag_defines ARGS((FILE* out,
                                           const SymbTable_ptr st,
                                           hash_ptr defines,
                                           hash_ptr obfuscation_map));

static node_ptr
compile_get_rid_of_define_chain ARGS((SymbTable_ptr st,
                                      node_ptr expr, hash_ptr cdh));

static node_ptr
__create_define_name ARGS((SymbTable_ptr st,
                           const char * prefix, node_ptr body));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_WriteFlattenModel(FILE* out,
                               const SymbTable_ptr st,
                               const array_t* layer_names,
                               const char* fsm_name,
                               FlatHierarchy_ptr hierarchy,
                               boolean force_flattening)
{
  /* these are used for making the tree a dag */
  hash_ptr dag_info = (hash_ptr) NULL;
  hash_ptr defines = (hash_ptr) NULL;
  /* to remove chain of defines */
  hash_ptr cdh = (hash_ptr) NULL;

  cdh = new_assoc();
  nusmv_assert((hash_ptr)NULL != cdh);

  RESET_DAG_HITS_NUMBER();

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    dag_info = compile_create_dag_info_from_hierarchy(st,
                                                      hierarchy,
                                                      SYMB_LAYER(NULL),
                                                      BDD_ENC(NULL),
                                                      force_flattening,
                                                      cdh);
    defines = new_assoc();
    nusmv_assert(defines != (hash_ptr) NULL);
  }

  /* dumps the FSM */
  compile_write_flat_fsm(out, st, layer_names, fsm_name, hierarchy,
                         dag_info, defines, force_flattening, cdh);

  /* dumps the specifications */
  compile_write_flat_specs(out, st, hierarchy, dag_info, defines,
                           force_flattening, cdh);

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    Compile_write_dag_defines(out, defines);
    PRINT_DAG_STATS();
    Compile_destroy_dag_info(dag_info, defines);
  }

  if ((hash_ptr) NULL != defines) free_assoc(defines);
  if ((hash_ptr) NULL != dag_info) free_assoc(dag_info);
  if ((hash_ptr) NULL != cdh) free_assoc(cdh);
}


/**Function********************************************************************

   Synopsis           [Dumps the flatten model on the given FILE]

   Description        [Dumps the flatten model on the given FILE.
                       The dumped model is restricted to the set of variables
                       defined in the given FlatHierarchy]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_WriteRestrictedFlattenModel(FILE* out,
                                         const SymbTable_ptr st,
                                         const array_t* layer_names,
                                         const char* fsm_name,
                                         FlatHierarchy_ptr hierarchy,
                                         boolean force_flattening)
{
  /* these are used for making the tree a dag */
  hash_ptr dag_info = (hash_ptr) NULL;
  hash_ptr defines = (hash_ptr) NULL;
  /* to remove chain of defines */
  hash_ptr cdh = (hash_ptr) NULL;

  cdh = new_assoc();
  nusmv_assert((hash_ptr)NULL != cdh);

  RESET_DAG_HITS_NUMBER();

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    dag_info = compile_create_dag_info_from_hierarchy(st,
                                                      hierarchy,
                                                      SYMB_LAYER(NULL),
                                                      BDD_ENC(NULL),
                                                      force_flattening,
                                                      cdh);
    defines = new_assoc();
    nusmv_assert((hash_ptr) NULL != defines);
  }

  /* dumps the FSM */
  compile_write_restricted_flat_fsm(out, st, layer_names, fsm_name, hierarchy,
                                    dag_info, defines, force_flattening,
                                    cdh);

  /* dumps the specifications */
  compile_write_flat_specs(out, st, hierarchy, dag_info, defines,
                           force_flattening, cdh);

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    Compile_write_dag_defines(out, defines);
    PRINT_DAG_STATS();
    Compile_destroy_dag_info(dag_info, defines);
  }

  if ((hash_ptr) NULL != defines) free_assoc(defines);
  if ((hash_ptr) NULL != dag_info) free_assoc(dag_info);
  if ((hash_ptr) NULL != cdh) free_assoc(cdh);
}

/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_WriteObfuscatedFlattenModel(FILE* out,
                                         const SymbTable_ptr st,
                                         const array_t* layer_names,
                                         const char* fsm_name,
                                         FlatHierarchy_ptr hierarchy,
                                         boolean print_map,
                                         boolean force_flattening)
{
  /* these are used for making the tree a dag */
  hash_ptr dag_info = (hash_ptr) NULL;
  hash_ptr defines = (hash_ptr) NULL;
  hash_ptr obfuscation_map;
  /* to remove chain of defines */
  hash_ptr cdh = (hash_ptr) NULL;

  cdh = new_assoc();
  nusmv_assert((hash_ptr)NULL != cdh);

  RESET_DAG_HITS_NUMBER();

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    dag_info = compile_create_dag_info_from_hierarchy(st,
                                                      hierarchy,
                                                      SYMB_LAYER(NULL),
                                                      BDD_ENC(NULL),
                                                      force_flattening,
                                                      cdh);
    defines = new_assoc();
    nusmv_assert(defines != (hash_ptr) NULL);
  }

  obfuscation_map = Compile_get_obfuscation_map(st);

  /* dumps the FSM */
  compile_write_obfuscated_flat_fsm(out,
                                    st,
                                    layer_names,
                                    fsm_name,
                                    hierarchy,
                                    dag_info,
                                    defines,
                                    obfuscation_map,
                                    force_flattening,
                                    cdh);

  /* dumps the specifications */
  compile_write_obfuscated_flat_specs(out,
                                      st,
                                      hierarchy,
                                      dag_info,
                                      defines,
                                      obfuscation_map,
                                      force_flattening,
                                      cdh);

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    compile_write_obfuscated_dag_defines(out, st, defines, obfuscation_map);
  }

  /* If requested, output the obfuscation map */
  if (print_map) {
    int i;
    node_ptr key, value;
    st_generator *gen;

    for (i=0; i<80; i++) fprintf(out, "-");
    fprintf(out,
           "\n\n-- The Obfuscation Key Map contains %d entries listed below:\n",
           st_count(obfuscation_map));

    st_foreach_item(obfuscation_map, gen, &key, &value) {
      fprintf(out, "--    ");
      print_node(out, value);
      fprintf(out, "\t<->\t");
      print_node(out, key);
      fprintf(out, "\n");
    }

    fprintf(out, "\n");
  }

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    PRINT_DAG_STATS();
    Compile_destroy_dag_info(dag_info, defines);
  }

  if ((hash_ptr) NULL != defines) free_assoc(defines);
  if ((hash_ptr) NULL != dag_info) free_assoc(dag_info);
  if ((hash_ptr) NULL != obfuscation_map) free_assoc(obfuscation_map);
  if ((hash_ptr)NULL != cdh) free_assoc(cdh);
}


/**Function********************************************************************

   Synopsis           [Prints the flatten version of FSM of an SMV model.]

   Description        [Prints on the specified file the flatten
   FSM of an SMV model, i.e. a list of all variable, defines, and all
   constrains (INIT, TRANS, INVAR, ASSIGNS, JUSTICE, COMPASSION).
   Specifications are NOT printed.

   layer_names is an array of names of layers that is typically
   obtained from the symbol table. fsm_name is a name of the output
   structure, usually it is "MODULE main".  ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_WriteFlattenFsm(FILE* out,
                             const SymbTable_ptr st,
                             const array_t* layer_names,
                             const char* fsm_name,
                             FlatHierarchy_ptr hierarchy,
                             boolean force_flattening)
{
  hash_ptr cdh = (hash_ptr)NULL;

  cdh = new_assoc();
  nusmv_assert((hash_ptr)NULL != cdh);

  compile_write_flat_fsm(out, st, layer_names, fsm_name, hierarchy,
                         (hash_ptr) NULL, (hash_ptr) NULL,
                         force_flattening, cdh);

  if ((hash_ptr)NULL != cdh) free_assoc(cdh);
}


/**Function********************************************************************

   Synopsis           [Prints the given flatten specifications.]

   Description        [Prints into the specified file the flatten
   specifications.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_WriteFlattenSpecs(FILE* out,
                               const SymbTable_ptr st,
                               FlatHierarchy_ptr hierarchy,
                               boolean force_flattening)
{

  hash_ptr cdh = (hash_ptr)NULL;

  cdh = new_assoc();
  nusmv_assert((hash_ptr)NULL != cdh);

  compile_write_flat_specs(out, st, hierarchy,
                           (hash_ptr) NULL, (hash_ptr) NULL,
                           force_flattening, cdh);

  if ((hash_ptr)NULL != cdh) free_assoc(cdh);
}



/**Function********************************************************************

   Synopsis           [Prints the given boolean model ]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_WriteBoolModel(FILE* out,
                            BddEnc_ptr enc,
                            NodeList_ptr layers,
                            const char* fsm_name,
                            BoolSexpFsm_ptr bool_sexp_fsm,
                            boolean force_flattening)
{
  FlatHierarchy_ptr fh;
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));
  SymbLayer_ptr det_layer;

  /* these are used for making the tree a dag */
  hash_ptr dag_info = (hash_ptr) NULL;
  hash_ptr defines = (hash_ptr) NULL;
  /* to remove chain of defines */
  hash_ptr cdh = (hash_ptr) NULL;

  cdh = new_assoc();
  nusmv_assert((hash_ptr)NULL != cdh);

  det_layer = SymbTable_create_layer(st, (char*) NULL, /*temp name*/
                                     SYMB_LAYER_POS_DEFAULT);

  NodeList_append(layers, (node_ptr) det_layer);

  fh = SexpFsm_get_hierarchy(SEXP_FSM(bool_sexp_fsm));

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    dag_info = compile_create_dag_info_from_hierarchy(st,
                                                      fh,
                                                      det_layer,
                                                      enc,
                                                      force_flattening,
                                                      cdh);
    defines = new_assoc();
    nusmv_assert(defines != (hash_ptr) NULL);
  }

  /* dumps the FSM */
  compile_write_bool_fsm(out, st, layers, fsm_name, bool_sexp_fsm,
                         dag_info, defines, force_flattening, cdh);

  compile_write_bool_specs(out, enc, det_layer, fh, dag_info, defines, cdh);

  if (opt_is_daggifier_enabled(OptsHandler_get_instance())) {
    Compile_write_dag_defines(out, defines);
    PRINT_DAG_STATS();
    Compile_destroy_dag_info(dag_info, defines);
  }

  if ((hash_ptr) NULL != defines) free_assoc(defines);
  if ((hash_ptr) NULL != dag_info) free_assoc(dag_info);
  if ((hash_ptr) NULL != cdh) free_assoc(cdh);

  SymbTable_remove_layer(st, det_layer);
}


/**Function********************************************************************

   Synopsis           [Prints the boolean FSM of an SMV model.]

   Description        [Prints into the specified file the boolean FSM of an
   SMV model.
   bool_sexp_fsm should be a boolean Sexp FSM.
   layer_names is an array of layers whose variables will be printed,
   usually this parameter is a list of all layers committed to enc. The array
   should be ended by a NULL element.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_WriteBoolFsm(FILE* out, const SymbTable_ptr st,
                          NodeList_ptr layers, const char* fsm_name,
                          BoolSexpFsm_ptr bool_sexp_fsm,
                          boolean force_flattening)
{
  hash_ptr cdh = (hash_ptr)NULL;

  cdh = new_assoc();
  nusmv_assert((hash_ptr)NULL != cdh);

  compile_write_bool_fsm(out, st, layers, fsm_name, bool_sexp_fsm,
                         (hash_ptr) NULL, (hash_ptr) NULL,
                         force_flattening, cdh);
  if ((hash_ptr)NULL != cdh) free_assoc(cdh);
}


/**Function********************************************************************

   Synopsis           [Prints the boolean specifications of an SMV model.]

   Description        [Prints into the specified file the booleanized
   specifications of an SMV model.

   NOTE: a temporary layer will be created during the dumping for
   determinization variables that derived from the booleanization of
   the specifications. These variable declarations will be printed
   after the specs.
   ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_WriteBoolSpecs(FILE* out,
                            BddEnc_ptr enc,
                            FlatHierarchy_ptr hierarchy)
{
  hash_ptr cdh = (hash_ptr)NULL;
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));
  SymbLayer_ptr det_layer;
  det_layer = SymbTable_create_layer(st, (char*) NULL, /*temp name*/
                                     SYMB_LAYER_POS_DEFAULT);

  cdh = new_assoc();
  nusmv_assert((hash_ptr)NULL != cdh);

  compile_write_bool_specs(out, enc, det_layer, hierarchy,
                           (hash_ptr) NULL, (hash_ptr) NULL, cdh);
  if ((hash_ptr)NULL != cdh) free_assoc(cdh);
  SymbTable_remove_layer(st, det_layer);
}

/**Function********************************************************************

   Synopsis           []

   Description        [Returns a node COLON(NUMBER count, NUMBER depth)]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr Compile_make_dag_info(node_ptr expr, hash_ptr hash)
{
  return compile_make_dag_info_aux(node_normalize(expr), hash);
}

/**Function********************************************************************

   Synopsis           [Frees the content of given structures.]

   Description        [Warning: the hashes are not freed, only the content]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_destroy_dag_info(hash_ptr dag_info, hash_ptr defines)
{
  clear_assoc_and_free_entries(dag_info, compile_free_node);
  clear_assoc_and_free_entries(defines, compile_free_define);
  defines_count = 0;
}

/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_write_dag_defines(FILE* out, hash_ptr defines)
{
  char *key, *value;
  st_generator *gen;
  boolean msg_printed = false;

  st_foreach_item(defines, gen, &key, &value) {
    node_ptr define = (node_ptr) value;
    nusmv_assert(define == Nil || node_get_type(define) == COLON);
    if (define != Nil) {
      unsigned int count = PTR_TO_INT(cdr(define));
      if (count > 0) {
        if (!msg_printed) {
          fprintf(out, "-- Symbols introduced by the dumper:\n");
          msg_printed = true;
        }
        if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
          fprintf(out, "-- occurrences: %d\n", count+1);
        }
        fprintf(out, "DEFINE ");
        print_node(out, car(define));
        fprintf(out, " ;\n\n");
      }
    }
  }
}


/**Function********************************************************************

   Synopsis           [Top level function to create dags from expressions]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr Compile_convert_to_dag(SymbTable_ptr symb_table,
                                node_ptr expr,
                                hash_ptr dag_hash,
                                hash_ptr defines)
{
  return compile_convert_to_dag_aux(symb_table,
                                   node_normalize(expr), dag_hash,
                                   opt_get_daggifier_counter_threshold(OptsHandler_get_instance()),
                                   opt_get_daggifier_depth_threshold(OptsHandler_get_instance()),
                                   defines,
                                   opt_traces_hiding_prefix(OptsHandler_get_instance()));
}

/**Function********************************************************************

   Synopsis           [Prints an array define node to out file.]

   Description        [Prints a array define node to out file.
   This function is exported so the hrc package can use it.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_print_array_define(FILE* out, const node_ptr n)
{
  node_ptr iter;
  switch (node_get_type(n)) {
  case ARRAY_DEF:
    nusmv_assert(Nil == cdr(n));

    fprintf(out, "[ ");

    for (iter = car(n); iter != Nil; iter = cdr(iter)) {
      nusmv_assert(CONS == node_get_type(iter));
      Compile_print_array_define(out, car(iter));
      if (cdr(iter) != Nil) fprintf(out, ", ");
    }
    fprintf(out, " ]");
    break;

  default:
    print_node(out, n);
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis    [Returns true iff this name is sub-element of
   an array define.]

   Description [If name refers to an array element the index has to be
   a NUMBER. The name has to be a define or array define identifier. ]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean
is_array_define_element(const SymbTable_ptr symb_table, const node_ptr name)
{
  /* See description of compileFlattenSexpRecur for docs about arrays. */

  /* array may have only NUMBER subscript */
  nusmv_assert(ARRAY != node_get_type(name) ||
               NUMBER == node_get_type(cdr(name)));

  /* Note that for
     DEFINE d := [1,2,3];
            d[4] := OK;
     d[4] is not part of d, but d[1] is.
  */
  if (ARRAY == node_get_type(name) &&
      SymbTable_is_symbol_array_define(symb_table, car(name))) {
    int val = node_get_int(cdr(name));
    node_ptr body = SymbTable_get_array_define_body(symb_table, car(name));
    nusmv_assert(ARRAY_DEF == node_get_type(body));
    int len = llength(car(body));
    if (val >= 0 && val < len) return true;
  }
  return false;
}


/**Function********************************************************************

   Synopsis    [Returns true iff this name is sub-element of
   a variable array.]

   Description [If name refers to an array element the index has to
   be a NUMBER. The name has to be a var or array var identifier.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean
is_array_var_element(const SymbTable_ptr symb_table, const node_ptr name)
{
  /* See description of compileFlattenSexpRecur for docs about arrays.
     ARRAY may be a part of identifier as well as index-subscript of
     an array . For the former, we print the identifier here. For the
     later we print the outer array but not the identifier.
  */

  /* array subscript can be only NUMBER */
  nusmv_assert(node_get_type(name) != ARRAY ||
               node_get_type(cdr(name)) == NUMBER);

  /* here we check if name[N] is a part of array name.
     Note that for
     VAR b: array 0..1 of boolean;
         b[4] : word[3];
     b[4] is not part of b, but b[1] is.
  */
  if (node_get_type(name) == ARRAY &&
      SymbTable_is_symbol_variable_array(symb_table, car(name))) {
    SymbType_ptr type = SymbTable_get_variable_array_type(symb_table, car(name));
    int val = node_get_int(cdr(name));
    if (SymbType_get_array_lower_bound(type) <= val &&
        SymbType_get_array_upper_bound(type) >= val) {
      return true;
    }
  }
  return false;
}


/**Function********************************************************************

   Synopsis    [Writes DEFINE declarations in SMV format on a file.]

   Description []

   SideEffects []

   SeeAlso     []

******************************************************************************/
static int compile_write_flat_define(const SymbTable_ptr symb_table, FILE* out,
                                     const NodeList_ptr names,
                                     hash_ptr dag_info, hash_ptr defines,
                                     boolean force_flattening)
{
  ListIter_ptr iter;
  hash_ptr printed_arrays;

  if (NodeList_get_length(names) == 0) return 1;

  fprintf(out, "DEFINE\n");
  printed_arrays = new_assoc();

  iter = NodeList_get_first_iter(names);
  while (! ListIter_is_end(iter)) {
    node_ptr name = NodeList_get_elem_at(names, iter);

    compile_write_flat_define_aux(symb_table, out,
                                  name,
                                  dag_info, defines,
                                  printed_arrays,
                                  force_flattening);
    iter = ListIter_get_next(iter);
  }

  free_assoc(printed_arrays);
  fprintf(out, "\n");
  return 1;
}


/**Function********************************************************************

   Synopsis    [Writes DEFINE declarations in SMV format on a file.]

   Description [This function behaves exactly like compile_write_flat_define
   except that identifiers a re obfuscated before.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static int compile_write_obfuscated_flat_define(const SymbTable_ptr symb_table,
                                                FILE* out,
                                                const SymbLayer_ptr layer,
                                                hash_ptr dag_info,
                                                hash_ptr defines,
                                                hash_ptr obfuscation_map,
                                                boolean force_flattening)
{
  SymbLayerIter iter;
  hash_ptr printed_arrays;

  if (SymbLayer_get_defines_num(layer) == 0) return 1;

  fprintf(out, "DEFINE\n");
  printed_arrays = new_assoc();

  SYMB_LAYER_FOREACH(layer, iter, STT_DEFINE) {
    node_ptr name = SymbLayer_iter_get_symbol(layer, &iter);

    compile_write_obfuscated_flat_define_aux(symb_table, out,
                                             name,
                                             dag_info, defines,
                                             printed_arrays,
                                             obfuscation_map,
                                             force_flattening);
  }

  free_assoc(printed_arrays);
  fprintf(out, "\n");
  return 1;
}


/**Function********************************************************************

   Synopsis    [Writes a DEFINE declarations in SMV format on a file.]

   Description [If a define happens to be an array define's element
   then array is output (and remembered in printed_arrays)
   instead of the original identifiers.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int
compile_write_flat_define_aux(const SymbTable_ptr symb_table,
                              FILE* out,
                              node_ptr name,
                              hash_ptr dag_info,
                              hash_ptr defines,
                              hash_ptr printed_arrays,
                              boolean force_flattening)
{
  /* MODIFY THIS FUNCTION ONLY TOGETHER WITH
     compile_write_obfuscated_flat_define_aux */

  if (is_array_define_element(symb_table, name)) {
    /* output the outer array but not this identifier */
    compile_write_flat_define_aux(symb_table, out,
                                  car(name),
                                  dag_info,
                                  defines,
                                  printed_arrays,
                                  force_flattening);
  }
  else {
    node_ptr fdef;
    /* this is a proper define or array define */
    if (SymbTable_is_symbol_define(symb_table, name)) {
      fdef = SymbTable_get_define_body(symb_table, name);
      node_ptr ctx = SymbTable_get_define_context(symb_table, name);
      if (force_flattening || Nil != ctx) {
        fdef = Compile_FlattenSexp(symb_table, fdef, ctx);
      }
    }
    else {
      nusmv_assert(SymbTable_is_symbol_array_define(symb_table, name));
      /* print array define only if not yet printed */
      if (Nil != find_assoc(printed_arrays, name)) return 1;
      insert_assoc(printed_arrays, name, NODE_PTR(1));

      fdef = SymbTable_get_array_define_body(symb_table, name);
      node_ptr ctx = SymbTable_get_array_define_context(symb_table, name);
      if (force_flattening || Nil != ctx) {
        fdef = Compile_FlattenSexp(symb_table, fdef, ctx);
      }
    }
    nusmv_assert(fdef != Nil);

    print_node(out, name);
    fprintf(out, " := ");

    /* get rid of chains of defines. useful only for normal defines */
    fdef = CompileFlatten_resolve_define_chains(symb_table, fdef, Nil);

    print_node(out, Compile_convert_to_dag(symb_table,
                                           fdef,
                                           dag_info,
                                           defines));
    fprintf(out, ";\n");
  }
  return 1;
}


/**Function********************************************************************

   Synopsis    [Writes a DEFINE declarations in SMV format on a file.]

   Description [This function behaves example like
   compile_write_flat_define_aux
   except that identifiers are obfuscated before being printed.
   ]

   SideEffects        []

   SeeAlso            [compile_write_flat_define_aux]

******************************************************************************/
static int
compile_write_obfuscated_flat_define_aux(const SymbTable_ptr symb_table,
                                         FILE* out,
                                         node_ptr name,
                                         hash_ptr dag_info,
                                         hash_ptr defines,
                                         hash_ptr printed_arrays,
                                         hash_ptr obfuscation_map,
                                         boolean force_flattening)
{
  /* MODIFY THIS FUNCTION ONLY TOGETHER WITH
     compile_write_flat_define_aux */

  if (is_array_define_element(symb_table, name)) {
    /* output the outer array but not this identifier */
    compile_write_obfuscated_flat_define_aux(symb_table, out,
                                             car(name),
                                             dag_info,
                                             defines,
                                             printed_arrays,
                                             obfuscation_map,
                                             force_flattening);
  }
  else {
    node_ptr fdef;
    /* this is a proper define or array define */
    if (SymbTable_is_symbol_define(symb_table, name)) {
      fdef = SymbTable_get_define_body(symb_table, name);
      node_ptr ctx = SymbTable_get_define_context(symb_table, name);
      if (force_flattening || Nil != ctx) {
        fdef = Compile_FlattenSexp(symb_table, fdef, ctx);
      }
    }
    else {
      nusmv_assert(SymbTable_is_symbol_array_define(symb_table, name));
      /* print array define only if not yet printed */
      if (Nil != find_assoc(printed_arrays, name)) return 1;
      insert_assoc(printed_arrays, name, NODE_PTR(1));

      fdef = SymbTable_get_array_define_body(symb_table, name);
      node_ptr ctx = SymbTable_get_array_define_context(symb_table, name);
      if (force_flattening || Nil != ctx) {
        fdef = Compile_FlattenSexp(symb_table, fdef, ctx);
      }
    }
    nusmv_assert(fdef != Nil);

    print_node(out, Compile_obfuscate_expression(symb_table,
                                                 name,
                                                 obfuscation_map));
    fprintf(out, " := ");

    /* get rid of chains of defines. useful only for normal defines */
    fdef = CompileFlatten_resolve_define_chains(symb_table, fdef, Nil);

    print_node(out, Compile_obfuscate_expression(symb_table,
                           Compile_convert_to_dag(symb_table, fdef,
                                                  dag_info, defines),
                                                     obfuscation_map));
    fprintf(out, ";\n");
  }
  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes VAR, FROZENVAR, and IVAR declarations in
   SMV format on a file.]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_flatten_vars(const SymbTable_ptr symb_table,
                                      FILE* out, const SymbLayer_ptr layer,
                                      SymbLayerIter* iter)
{
  hash_ptr printed_arrays;
  unsigned int count = 0;

  enum { WFV_UNKNOWN, WFV_INPUT, WFV_STATE, WFV_FROZEN } last_insert = WFV_UNKNOWN;

  printed_arrays = new_assoc();

  while (!SymbLayer_iter_is_end(layer, iter)) {
    node_ptr name = SymbLayer_iter_get_symbol(layer, iter);

    count ++;

    if (SymbTable_is_symbol_var(symb_table, name)) {

      if (SymbTable_is_symbol_state_var(symb_table, name)
          && last_insert != WFV_STATE) {
        fprintf(out, "VAR\n");
        last_insert = WFV_STATE;
      }
      else if (SymbTable_is_symbol_frozen_var(symb_table, name)
               && last_insert != WFV_FROZEN) {
        fprintf(out, "FROZENVAR\n");
        last_insert = WFV_FROZEN;
      }
      else if (SymbTable_is_symbol_input_var(symb_table, name)
               &&
               last_insert != WFV_INPUT) {
        fprintf(out, "IVAR\n");
        last_insert = WFV_INPUT;
      }
      else {
        nusmv_assert(last_insert != WFV_UNKNOWN);
      }
      /* print the var declaration */
      compile_write_flatten_vars_aux(symb_table, name, out,
                                     printed_arrays);
    }

    SymbLayer_iter_next(layer, iter);
  } /* loop */

  /* Destroy the printed arrays assoc */
  free_assoc(printed_arrays);

  if (0 == count) return 0;

  fprintf(out, "\n");
  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes VAR, FROZENVAR, and IVAR declarations in
   SMV format on a file.]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_obfuscated_flatten_vars(const SymbTable_ptr symb_table,
                                                 FILE* out,
                                                 const SymbLayer_ptr layer, 
                                                 const SymbTableType type,
                                                 hash_ptr obfuscation_map)
{
  SymbLayerIter iter;
  hash_ptr printed_arrays;

  enum { WFV_UNKNOWN, WFV_INPUT, WFV_STATE, WFV_FROZEN } last_insert = WFV_UNKNOWN;

  printed_arrays = new_assoc();

  SYMB_LAYER_FOREACH(layer, iter, type) {
    node_ptr name = SymbLayer_iter_get_symbol(layer, &iter);

    if (SymbTable_is_symbol_var(symb_table, name)) {

      if (SymbTable_is_symbol_state_var(symb_table, name)
          && last_insert != WFV_STATE) {
        fprintf(out, "VAR\n");
        last_insert = WFV_STATE;
      }
      else if (SymbTable_is_symbol_frozen_var(symb_table, name)
               && last_insert != WFV_FROZEN) {
        fprintf(out, "FROZENVAR\n");
        last_insert = WFV_FROZEN;
      }
      else if (SymbTable_is_symbol_input_var(symb_table, name)
               &&
               last_insert != WFV_INPUT) {
        fprintf(out, "IVAR\n");
        last_insert = WFV_INPUT;
      }
      else {
        nusmv_assert(last_insert != WFV_UNKNOWN);
      }

      /* print the var declaration */
      compile_write_obfuscated_flatten_vars_aux(symb_table,
                                                name,
                                                out,
                                                printed_arrays,
                                                obfuscation_map);
    }

  } /* loop */

  /* Destroy the printed arrays assoc */
  free_assoc(printed_arrays);

  fprintf(out, "\n");
  return 1;
}


/**Function********************************************************************

   Synopsis    [Print the variable declaration.]

   Description [If the identifier contains an index subscript in its
   name then at first the identifier check for being a part of an array.
   In this case array is output (and remembered in "printed") instead of
   the var. Otherwise, the identifier is output.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static int compile_write_flatten_vars_aux(const SymbTable_ptr symb_table,
                                           const node_ptr name,
                                           FILE* out,
                                           hash_ptr printed)
{
  /* MODIFY THIS FUNCTION ONLY TOGETHER WITH
     compile_write_obfuscated_flatten_vars_aux */

  if (is_array_var_element(symb_table, name)) {
      /* this identifier is a subpart of array => print the outer array only */
      compile_write_flatten_vars_aux(symb_table, car(name), out, printed);
  }
  else {
    /* this identifier is an array or an individual identifier
         declared with index subscript in its name */
    if (SymbTable_is_symbol_variable_array(symb_table, name)) {
      if (find_assoc(printed, name) == Nil) {
        /* print array only if not printed yet */
        SymbType_ptr type = SymbTable_get_variable_array_type(symb_table, name);
        nusmv_assert(type != (SymbType_ptr) NULL);

        print_node(out, name);
        fprintf(out, " : ");
        SymbType_print(type, out);
        fprintf(out, ";\n");
        insert_assoc(printed, name, (node_ptr) type);
      }
    }
    else { /* this is a normal variable */
      print_node(out, name);
      fprintf(out, " : ");
      SymbType_print(SymbTable_get_var_type(symb_table, name), out);
      fprintf(out, ";\n"); /* end of the variable output */
    }
  }
  return 1;
}


/**Function********************************************************************

   Synopsis    [Print the variable declaration after obfuscation]

   Description [The function works exactly like
   compile_write_flatten_vars_aux but all identifiers
   are obfuscated before being printed.]

   SideEffects []

   SeeAlso     [compile_write_flatten_vars_aux]

******************************************************************************/
static boolean
compile_write_obfuscated_flatten_vars_aux(const SymbTable_ptr symb_table,
                                          const node_ptr name,
                                          FILE* out,
                                          hash_ptr printed,
                                          hash_ptr obfuscation_map)
{
  /* MODIFY THIS FUNCTION ONLY TOGETHER WITH
     compile_write_flatten_vars_aux */

  if (is_array_var_element(symb_table, name)) {
    /* this identifier is a subpart of array => print the outer array only */
    return compile_write_obfuscated_flatten_vars_aux(symb_table,
                                                     car(name),
                                                     out,
                                                     printed,
                                                     obfuscation_map);
  }
  else {
    /* this identifier is an array or an individual identifier
       declared with index subscript in its name */
    if (SymbTable_is_symbol_variable_array(symb_table, name)) {
      if (find_assoc(printed, name) == Nil) {
        /* print array only if not printed yet */
        SymbType_ptr type = SymbTable_get_variable_array_type(symb_table, name);
        nusmv_assert(type != (SymbType_ptr) NULL);

        print_node(out,
                   Compile_obfuscate_expression(symb_table, name, obfuscation_map));
        fprintf(out, " : ");
        compile_symbtype_obfuscated_print(type, out, symb_table,
                                          obfuscation_map);
        fprintf(out, ";\n");
        insert_assoc(printed, name, (node_ptr) type);
      }
    }
    else { /* this is a normal variable */
      print_node(out, Compile_obfuscate_expression(symb_table,
                                                   name,
                                                   obfuscation_map));
      fprintf(out, " : ");
      compile_symbtype_obfuscated_print(SymbTable_get_var_type(symb_table,
                                                               name),
                                        out,
                                        symb_table,
                                        obfuscation_map);
      fprintf(out, ";\n"); /* end of the variable output */
    }
  }

  return true;
}



/**Function********************************************************************

   Synopsis           [Apply the obfuscation over an expression]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr Compile_obfuscate_expression(const SymbTable_ptr symb_table,
                                      node_ptr expr,
                                      const hash_ptr obfuscation_map) {
  ResolveSymbol_ptr rs;

  if (Nil == expr) {
    return expr;
  }

  /* Try to resolve names */
  {
    node_ptr resolved;

    rs = SymbTable_resolve_symbol(symb_table, expr, Nil);
    resolved = ResolveSymbol_get_resolved_name(rs);

    /* expr is not identifier but expression */
    if (Nil != resolved && TYPE_ERROR != resolved) {
      expr = resolved;
    }
  }

  if (ResolveSymbol_is_defined(rs)) {
    /* this is an identifier */
    node_ptr res = find_assoc(obfuscation_map, expr);

    if (Nil == res) {
      /* the only possibilities for an identifier to be declared but not
         be in obfuscation map is to be :
         1. a constant
         2. element of array var or array define */
      if (ARRAY == node_get_type(expr)) {
        res = find_node(ARRAY,
                        Compile_obfuscate_expression(symb_table,
                                                     car(expr),
                                                     obfuscation_map),
                        Compile_obfuscate_expression(symb_table,
                                                     cdr(expr),
                                                     obfuscation_map));
      }
      else {
        res = expr;
      }
    }
    return res;
  }
  else {
    /* If we reached a leaf */
    if ((node_is_leaf(expr)) ||
        (ATOM == node_get_type(expr)) ||
        (DOT == node_get_type(expr))) {
      return expr;
    }
    else {
      return find_node(node_get_type(expr),
                       Compile_obfuscate_expression(symb_table,
                                                    car(expr),
                                                    obfuscation_map),
                       Compile_obfuscate_expression(symb_table,
                                                    cdr(expr),
                                                    obfuscation_map));
    }
  }

  error_unreachable_code();
}


/**Function********************************************************************

   Synopsis           [Writes flattened ASSIGN declarations in SMV format on a
   file.]

   Description        [Writes flattened ASSIGN declarations in SMV format on a
   file.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_flat_asgn(const SymbTable_ptr symb_table,
                                   FILE* out, const NodeList_ptr vars,
                                   FlatHierarchy_ptr hierarchy,
                                   hash_ptr dag_info, hash_ptr defines,
                                   hash_ptr cdh)
{
  ListIter_ptr iter;

  NODE_LIST_FOREACH(vars, iter) {
    node_ptr name = NodeList_get_elem_at(vars, iter);
    node_ptr init_name = find_node(SMALLINIT, name, Nil);
    node_ptr next_name = find_node(NEXT, name, Nil);
    node_ptr invar_expr = compile_get_rid_of_define_chain(symb_table,
                            FlatHierarchy_lookup_assign(hierarchy, name),
                            cdh);
    node_ptr init_expr = compile_get_rid_of_define_chain(symb_table,
                            FlatHierarchy_lookup_assign(hierarchy, init_name),
                            cdh);
    node_ptr next_expr = compile_get_rid_of_define_chain(symb_table,
                            FlatHierarchy_lookup_assign(hierarchy, next_name),
                            cdh);

    if ((init_expr != (node_ptr) NULL) ||
        (next_expr != (node_ptr) NULL) ||
        (invar_expr != (node_ptr) NULL)) {
      fprintf(out, "ASSIGN\n");
    }

    if (init_expr != (node_ptr) NULL) compile_print_assign(symb_table,
                                                           out,
                                                           init_name,
                                                           init_expr,
                                                           dag_info,
                                                           defines);
    if (invar_expr != (node_ptr) NULL) compile_print_assign(symb_table,
                                                            out,
                                                            name,
                                                            invar_expr,
                                                            dag_info,
                                                            defines);
    if (next_expr != (node_ptr) NULL) compile_print_assign(symb_table,
                                                           out,
                                                           next_name,
                                                           next_expr,
                                                           dag_info,
                                                           defines);

    if ((init_expr != (node_ptr)NULL) ||
        (next_expr != (node_ptr)NULL) ||
        (invar_expr != (node_ptr)NULL)) {
      fprintf(out, "\n");
    }
  }

  fprintf(out, "\n");
  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened ASSIGN declarations in SMV format on a
   file.]

   Description        [Writes flattened ASSIGN declarations in SMV format on a
   file.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_obfuscated_flat_asgn(const SymbTable_ptr symb_table,
                                              FILE* out,
                                              const SymbLayer_ptr layer,
                                              FlatHierarchy_ptr hierarchy,
                                              hash_ptr dag_info,
                                              hash_ptr defines,
                                              hash_ptr obfuscation_map,
                                              hash_ptr cdh)
{
  SymbLayerIter iter;

  SYMB_LAYER_FOREACH(layer, iter, STT_VAR) {
    node_ptr name = SymbLayer_iter_get_symbol(layer, &iter);
    node_ptr init_name = find_node(SMALLINIT, name, Nil);
    node_ptr next_name = find_node(NEXT, name, Nil);
    node_ptr invar_expr = Compile_obfuscate_expression(symb_table,
                             compile_get_rid_of_define_chain(symb_table,
                                FlatHierarchy_lookup_assign(hierarchy, name),
                                                       cdh), obfuscation_map);
    node_ptr init_expr = Compile_obfuscate_expression(symb_table,
                            compile_get_rid_of_define_chain(symb_table,
                               FlatHierarchy_lookup_assign(hierarchy, init_name),
                                                       cdh), obfuscation_map);
    node_ptr next_expr = Compile_obfuscate_expression(symb_table,
                            compile_get_rid_of_define_chain(symb_table,
                               FlatHierarchy_lookup_assign(hierarchy, next_name),
                                                       cdh), obfuscation_map);

    init_name = Compile_obfuscate_expression(symb_table,
                                             init_name,
                                             obfuscation_map);
    next_name = Compile_obfuscate_expression(symb_table,
                                             next_name,
                                             obfuscation_map);
    name = Compile_obfuscate_expression(symb_table,
                                        name,
                                        obfuscation_map);

    if ((init_expr != (node_ptr) NULL) ||
        (next_expr != (node_ptr) NULL) ||
        (invar_expr != (node_ptr) NULL)) {
      fprintf(out, "ASSIGN\n");
    }

    if (init_expr != (node_ptr) NULL) compile_print_assign(symb_table,
                                                           out,
                                                           init_name,
                                                           init_expr,
                                                           dag_info,
                                                           defines);
    if (invar_expr != (node_ptr) NULL) compile_print_assign(symb_table,
                                                            out,
                                                            name,
                                                            invar_expr,
                                                            dag_info,
                                                            defines);
    if (next_expr != (node_ptr) NULL) compile_print_assign(symb_table,
                                                           out,
                                                           next_name,
                                                           next_expr,
                                                           dag_info,
                                                           defines);

    if ((init_expr != (node_ptr)NULL) ||
        (next_expr != (node_ptr)NULL) ||
        (invar_expr != (node_ptr)NULL)) {
      fprintf(out, "\n");
    }
  }

  fprintf(out, "\n");
  return 1;
}

/**Function********************************************************************

   Synopsis           [Prints an assignement statement]

   Description        [Prints an assignement statement]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_print_assign(SymbTable_ptr st,
                                 FILE * out,
                                 node_ptr lhs,
                                 node_ptr rhs,
                                 hash_ptr dag_info,
                                 hash_ptr defines)
{
  print_node(out, lhs);
  fprintf(out, " := ");
  print_node(out, Compile_convert_to_dag(st, rhs, dag_info, defines));
  fprintf(out, ";\n");
}


/**Function********************************************************************

   Synopsis           [Writes boolean VAR, FROZENVAR and IVAR declarations in
   SMV format on a file. Non boolean vars are dumped as defines for the sake of
   readability of conterexamples. ]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_flatten_bool_vars(const SymbTable_ptr symb_table,
                                           const BoolEnc_ptr bool_enc,
                                           FILE* out, const SymbLayer_ptr layer, 
                                           const SymbTableType type)
{
  SymbLayerIter iter;
  enum { WFV_UNKNOWN, WFV_DEFINE,
         WFV_INPUT, WFV_STATE, WFV_FROZEN } last_insert = WFV_UNKNOWN;

  SYMB_LAYER_FOREACH(layer, iter, type) {
    node_ptr name = SymbLayer_iter_get_symbol(layer, &iter);

    if (SymbTable_is_symbol_var(symb_table, name)) {
      if (!SymbTable_is_symbol_bool_var(symb_table, name)) {
        if (!opt_backward_comp(OptsHandler_get_instance())) {
          /* dumps the scalar variable as a define */
          node_ptr body;
          if (last_insert != WFV_DEFINE) {
            fprintf(out, "DEFINE\n");
            last_insert = WFV_DEFINE;
          }
          print_node(out, name);
          fprintf(out, " := ");
          body = BoolEnc_get_var_encoding(bool_enc, name);
          print_node(out, body);
          fprintf(out, ";\n");
        }
      }

      else {
        /* dumps it as boolean var */
        if (SymbTable_is_symbol_state_var(symb_table, name)
            && last_insert != WFV_STATE) {
          fprintf(out, "VAR\n");
          last_insert = WFV_STATE;
        }
        else if (SymbTable_is_symbol_frozen_var(symb_table, name)
                 && last_insert != WFV_FROZEN) {
          fprintf(out, "FROZENVAR\n");
          last_insert = WFV_FROZEN;
        }
        else if (SymbTable_is_symbol_input_var(symb_table, name)
                 &&
                 last_insert != WFV_INPUT) {
          fprintf(out, "IVAR\n");
          last_insert = WFV_INPUT;
        }

        print_node(out, name);
        fprintf(out, " : ");
        SymbType_print(SymbTable_get_var_type(symb_table, name), out);
        fprintf(out, ";\n"); /* end of the variable output */
      }
    }
  } /* loop */

  fprintf(out, "\n");
  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened spec in SMV format on a file.]

   Description        [Writes a generic spec prefixed by a given
   string in SMV format on a file.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int
compile_write_flatten_spec_split(const SymbTable_ptr symb_table,
                                 FILE* out, node_ptr n, const char* s,
                                 hash_ptr dag_info, hash_ptr defines,
                                 boolean force_flattening, hash_ptr cdh)
{
  if (n == Nil) return 0;

  switch (node_get_type(n)) {
  case CONS:
  case AND:
    compile_write_flatten_spec_split(symb_table, out, car(n), s,
                                     dag_info, defines, force_flattening, cdh);

    compile_write_flatten_spec_split(symb_table, out, cdr(n), s,
                                     dag_info, defines, force_flattening, cdh);
    break;

  default:
    compile_write_flatten_spec(symb_table, out, n, s, dag_info, defines,
                               force_flattening, cdh);
  } /* switch */

  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened spec in SMV format on a file.]

   Description        [Writes a generic spec prefixed by a given
   string in SMV format on a file.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int
compile_write_obfuscated_flatten_spec_split(const SymbTable_ptr symb_table,
                                            FILE* out,
                                            node_ptr n,
                                            const char* s,
                                            hash_ptr dag_info,
                                            hash_ptr defines,
                                            hash_ptr obfuscation_map,
                                            boolean force_flattening,
                                            hash_ptr cdh)
{
  if (n == Nil) return 0;

  switch (node_get_type(n)) {
  case CONS:
  case AND:
    compile_write_obfuscated_flatten_spec_split(symb_table,
                                                out,
                                                car(n),
                                                s,
                                                dag_info,
                                                defines,
                                                obfuscation_map,
                                                force_flattening,
                                                cdh);

    compile_write_obfuscated_flatten_spec_split(symb_table,
                                                out,
                                                cdr(n),
                                                s,
                                                dag_info,
                                                defines,
                                                obfuscation_map,
                                                force_flattening,
                                                cdh);
    break;

  default:
    compile_write_obfuscated_flatten_spec(symb_table,
                                          out,
                                          n,
                                          s,
                                          dag_info,
                                          defines,
                                          obfuscation_map,
                                          force_flattening,
                                          cdh);
  } /* switch */

  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened spec in SMV format on a file.]

   Description        [Writes a generic spec prefixed by a given
   string in SMV format on a file.
   Returns true if at least one character was printed, and false otherwise.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_flatten_spec(const SymbTable_ptr symb_table,
                                      FILE* out, node_ptr n, const char* s,
                                      hash_ptr dag_info, hash_ptr defines,
                                      boolean force_flattening, hash_ptr cdh)
{
  if (n == Nil || Expr_is_true(n)) return 0;

  nusmv_assert((SPEC == node_get_type(n)) ||
               (LTLSPEC == node_get_type(n)) ||
               (INVARSPEC == node_get_type(n)) ||
               (PSLSPEC == node_get_type(n)) ||
               (COMPUTE == node_get_type(n)));

  node_ptr expr = car(n);
  node_ptr name = cdr(n);

  fprintf(out, "%s ", s);

  /* Support for property Names: Old property structure is in car(n),
     property name is in cdr(n).  */
  if (Nil != name){
    fprintf(out, "NAME ");
    print_node(out, name);
    fprintf(out, " := ");
  }

  /* flatten only if required, i.e. there is explicit context */
  if (CONTEXT == node_get_type(expr)) {
    if (force_flattening || (Nil != car(expr))) {
      expr = Compile_FlattenSexp(symb_table, cdr(expr), car(expr));
    }
  }

  print_node(out, Compile_convert_to_dag(symb_table, expr, dag_info, defines));
  fprintf(out, ";\n\n");
  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened spec in SMV format on a file.]

   Description        [Writes a generic spec prefixed by a given
   string in SMV format on a file.
   Returns true if at least one character was printed, and false otherwise.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_obfuscated_flatten_spec(const SymbTable_ptr symb_table,
                                                 FILE* out, node_ptr n,
                                                 const char* s,
                                                 hash_ptr dag_info,
                                                 hash_ptr defines,
                                                 hash_ptr obfuscation_map,
                                                 boolean force_flattening,
                                                 hash_ptr cdh)
{
  if (n == Nil || Expr_is_true(n)) return 0;

  nusmv_assert((SPEC == node_get_type(n)) ||
               (LTLSPEC == node_get_type(n)) ||
               (INVARSPEC == node_get_type(n)) ||
               (PSLSPEC == node_get_type(n)) ||
               (COMPUTE == node_get_type(n)));

  node_ptr expr = car(n);
  node_ptr name = cdr(n);

  fprintf(out, "%s ", s);

  /* Support for property Names: Old property structure is in car(n),
     property name is in cdr(n).  */
  if (Nil != name){
    fprintf(out, "NAME ");
    print_node(out, name);
    fprintf(out, " := ");
  }

  /* flatten only if required, i.e. there is explicit context */
  if (CONTEXT == node_get_type(expr)) {
    if(force_flattening || (Nil != car(expr))) {
      expr = Compile_FlattenSexp(symb_table, cdr(expr), car(expr));
    }
  }

  print_node(out, Compile_obfuscate_expression(symb_table,
                     Compile_convert_to_dag(symb_table,
                                            expr,
                                            dag_info,
                                            defines),
                                               obfuscation_map));
  fprintf(out, "\n\n");
  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened expression in SMV format on a file.]

   Description        [Writes a generic expression prefixed by a given
   string in SMV format on a file.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int
compile_write_obfuscated_flatten_expr_split(const SymbTable_ptr symb_table,
                                            FILE* out,
                                            node_ptr n,
                                            const char* s,
                                            hash_ptr dag_info,
                                            hash_ptr defines,
                                            hash_ptr obfuscation_map,
                                            boolean force_flattening,
                                            hash_ptr cdh)
{
  if (n == Nil) return 0;

  switch (node_get_type(n)) {
  case CONS:
  case AND:
    compile_write_obfuscated_flatten_expr_split(symb_table,
                                                out,
                                                car(n),
                                                s,
                                                dag_info,
                                                defines,
                                                obfuscation_map,
                                                force_flattening,
                                                cdh);

    compile_write_obfuscated_flatten_expr_split(symb_table,
                                                out,
                                                cdr(n),
                                                s,
                                                dag_info,
                                                defines,
                                                obfuscation_map,
                                                force_flattening,
                                                cdh);
    break;

  default:
    compile_write_obfuscated_flatten_expr(symb_table,
                                          out,
                                          n,
                                          s,
                                          dag_info,
                                          defines,
                                          obfuscation_map,
                                          force_flattening,
                                          cdh);
  } /* switch */

  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened expression in SMV format on a file.]

   Description        [Writes a generic expression prefixed by a given
   string in SMV format on a file.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int
compile_write_flatten_expr_split(const SymbTable_ptr symb_table,
                                 FILE* out, node_ptr n, const char* s,
                                 hash_ptr dag_info, hash_ptr defines,
                                 boolean force_flattening, hash_ptr cdh)
{
  if (n == Nil) return 0;

  switch (node_get_type(n)) {
  case CONS:
  case AND:
    compile_write_flatten_expr_split(symb_table, out, car(n), s,
                                     dag_info, defines, force_flattening,
                                     cdh);

    compile_write_flatten_expr_split(symb_table, out, cdr(n), s,
                                     dag_info, defines, force_flattening,
                                     cdh);
    break;

  default:
    compile_write_flatten_expr(symb_table, out, n, s, dag_info, defines,
                               force_flattening, cdh);
  } /* switch */

  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened expression in SMV format on a file.]

   Description        [Writes a generic expression prefixed by a given
   string in SMV format on a file.
   Returns true if at least one character was printed, and false otherwise.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_flatten_expr(const SymbTable_ptr symb_table,
                                      FILE* out, node_ptr n, const char* s,
                                      hash_ptr dag_info, hash_ptr defines,
                                      boolean force_flattening,
                                      hash_ptr cdh)
{
  if (n == Nil || (node_get_type(n) == TRUEEXP &&
                   /* this check is optimization */
                   n == find_node(TRUEEXP, Nil, Nil))) return 0;

  /* flatten only if required, i.e. there is explicit context */
  if (CONTEXT == node_get_type(n)) {
    if (force_flattening || (Nil != car(n))) {
      n = Compile_FlattenSexp(symb_table, cdr(n), car(n));
    }
  }
  {
    node_ptr n1 = compile_get_rid_of_define_chain(symb_table, n, cdh);
    if (n != n1) n = n1;
  }
  fprintf(out, "%s ", s);
  print_node(out, Compile_convert_to_dag(symb_table, n, dag_info, defines));
  fprintf(out, "\n\n");
  return 1;
}

/**Function********************************************************************

   Synopsis           [Writes flattened expression in SMV format on a file.]

   Description        [Writes a generic expression prefixed by a given
   string in SMV format on a file.
   Returns true if at least one character was printed, and false otherwise.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_obfuscated_flatten_expr(const SymbTable_ptr symb_table,
                                                 FILE* out,
                                                 node_ptr n,
                                                 const char* s,
                                                 hash_ptr dag_info,
                                                 hash_ptr defines,
                                                 hash_ptr obfuscation_map,
                                                 boolean force_flattening,
                                                 hash_ptr cdh)
{
  if (n == Nil || (node_get_type(n) == TRUEEXP &&
                   /* this check is optimization */
                   n == find_node(TRUEEXP, Nil, Nil))) return 0;

  /* flatten only if required, i.e. there is explicit context */
  if (CONTEXT == node_get_type(n)) {
    if(force_flattening || (Nil != car(n))) {
      n = Compile_FlattenSexp(symb_table, cdr(n), car(n));
    }
  }
  {
    node_ptr n1 = compile_get_rid_of_define_chain(symb_table, n, cdh);
    if (n != n1) n = n1;
  }
  fprintf(out, "%s ", s);
  print_node(out, Compile_obfuscate_expression(symb_table,
                     Compile_convert_to_dag(symb_table,
                                            n,
                                            dag_info,
                                            defines),
                                               obfuscation_map));
  fprintf(out, "\n\n");
  return 1;
}

/**Function********************************************************************

   Synopsis           [Writes PSL properties as they are.]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_flatten_psl(const SymbTable_ptr symb_table,
                                     FILE* out, node_ptr n,
                                     hash_ptr dag_info, hash_ptr defines,
                                     hash_ptr cdh)
{
  if (n == Nil) return 0;

  switch (node_get_type(n)) {
  case CONS:
  case AND:
    compile_write_flatten_psl(symb_table, out, car(n),
                              dag_info, defines, cdh);
    compile_write_flatten_psl(symb_table, out, cdr(n),
                              dag_info, defines, cdh);
    break;

  default:
    {

      nusmv_assert(PSLSPEC == node_get_type(n));

      node_ptr expr = car(n);
      node_ptr name = cdr(n);
      node_ptr dagged = Compile_convert_to_dag(symb_table,
                                               expr,
                                               dag_info,
                                               defines);

      fprintf(out, "-- PSLSPEC\n--   ");

      /* Support for property names */
      if (Nil != name){
        fprintf(out, "NAME ");
        print_node(out, name);
        fprintf(out, " := ");
      }

      print_node(out, dagged);
      fprintf(out, "\n\n");
    }
  } /* switch */

  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes flattened expression in SMV format on a file.]

   Description        [Writes a generic expression prefixed by a given
   string in SMV format on a file. The given layer is intended to hold the
   determization variables that are created by the booleanization process of
   the properties, that are kept not booleanized within the system.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_flatten_bfexpr(BddEnc_ptr enc,
                                        const SymbTable_ptr symb_table,
                                        SymbLayer_ptr det_layer,
                                        FILE* out, node_ptr n, const char* s,
                                        hash_ptr dag_info, hash_ptr defines,
                                        hash_ptr cdh)
{
  /* Nil and TRUEEXP indicate the end of a list */
  if (n == Nil || Expr_is_true(n)) return 0;

  switch (node_get_type(n)) {
  case CONS:
  case AND:
    compile_write_flatten_bfexpr(enc, symb_table, det_layer, out, car(n), s,
                                 dag_info, defines, cdh);
    compile_write_flatten_bfexpr(enc, symb_table, det_layer, out, cdr(n), s,
                                 dag_info, defines, cdh);
    break;

  default:
    {
      /* Support for property names */
      nusmv_assert(SPEC == node_get_type(n) ||
                   LTLSPEC == node_get_type(n) ||
                   INVARSPEC == node_get_type(n) ||
                   PSLSPEC == node_get_type(n) ||
                   COMPUTE == node_get_type(n));

      node_ptr expr = car(n);
      node_ptr name = cdr(n);

      /* specifications are wrapped into CONTEXT during hierarchy creation */
      nusmv_assert(CONTEXT == node_get_type(expr));

      /* booleanized property before printing */
      fprintf(out, "\n%s", s);

      if (Nil != name){
        fprintf(out, "NAME ");
        print_node(out, name);
        fprintf(out, " := ");
      }

      expr = Compile_convert_to_dag(symb_table,
                                    Compile_expr2bexpr(enc, det_layer, expr),
                                    dag_info, defines);
      print_node(out, expr);
      fprintf(out, "\n");
    }
  } /* switch */

  return 1;
}


/**Function********************************************************************

   Synopsis [Writes flattened expression pairs in SMV format on a
   file.]

   Description [Writes a list of flattened expression pairs prefixed by
   a given string in SMV format on a file.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_flatten_expr_pair(const SymbTable_ptr symb_table,
                                           FILE* out, node_ptr l, const char* s,
                                           hash_ptr dag_info, hash_ptr defines,
                                           boolean force_flattening,
                                           hash_ptr cdh)
{
  if (l == Nil) return 0;

  while (l) {
    node_ptr n = car(l);
    l = cdr(l);
    nusmv_assert(node_get_type(n) == CONS);

    fprintf(out, "%s (", s);

    if (node_get_type(n) == CONTEXT) {
      node_ptr fn;

      fn = n;
      if (force_flattening || (Nil != car(n))) {
        /* flats the context */
        fn = Compile_FlattenSexp(symb_table, car(n), Nil);
      }
      fn = Compile_convert_to_dag(symb_table, fn, dag_info, defines);
      print_node(out, fn);
    }
    else print_node(out, Compile_convert_to_dag(symb_table,
                                                car(n),
                                                dag_info,
                                                defines));

    fprintf(out, ", ");
    if (node_get_type(n) == CONTEXT) {
      node_ptr fn;

      fn = n;
      if (force_flattening || (Nil != car(n))) {
        /* flats the definition */
        fn = Compile_FlattenSexp(symb_table, cdr(n), Nil);
      }
      fn = Compile_convert_to_dag(symb_table, fn, dag_info, defines);
      print_node(out, fn);
    }
    else print_node(out, Compile_convert_to_dag(symb_table,
                                                cdr(n),
                                                dag_info,
                                                defines));

    fprintf(out, ")\n\n");
  }
  return 1;
}


/**Function********************************************************************

   Synopsis [Writes flattened expression pairs in SMV format on a
   file.]

   Description [Writes a list of flattened expression pairs prefixed by
   a given string in SMV format on a file.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int
compile_write_obfuscated_flatten_expr_pair(const SymbTable_ptr symb_table,
                                           FILE* out,
                                           node_ptr l,
                                           const char* s,
                                           hash_ptr dag_info,
                                           hash_ptr defines,
                                           hash_ptr obfuscation_map,
                                           boolean force_flattening,
                                           hash_ptr cdh)
{
  if (l == Nil) return 0;

  while (l) {
    node_ptr n = car(l);
    l = cdr(l);
    nusmv_assert(node_get_type(n) == CONS);

    fprintf(out, "%s (", s);

    if (node_get_type(n) == CONTEXT) {
      node_ptr fn;

      fn = n;
      if (force_flattening || (Nil != car(n))) {
        /* flats the context */
        fn = Compile_FlattenSexp(symb_table, car(n), Nil);
      }
      fn = Compile_convert_to_dag(symb_table, fn, dag_info, defines);
      fn = Compile_obfuscate_expression(symb_table, fn, obfuscation_map);
      print_node(out, fn);
    }
    else print_node(out,
                    Compile_obfuscate_expression(symb_table,
                       Compile_convert_to_dag(symb_table,
                                              car(n),
                                              dag_info,
                                              defines),
                                                 obfuscation_map));

    fprintf(out, ", ");
    if (node_get_type(n) == CONTEXT) {
      node_ptr fn;

      fn = n;
      if (force_flattening || (Nil != car(n))) {
        /* flats the definition */
        fn = Compile_FlattenSexp(symb_table, cdr(n), Nil);
      }
      fn = Compile_convert_to_dag(symb_table, fn, dag_info, defines);
      fn = Compile_obfuscate_expression(symb_table, fn, obfuscation_map);
      print_node(out, fn);
    }
    else print_node(out, Compile_obfuscate_expression(symb_table,
                            Compile_convert_to_dag(symb_table,
                                                   cdr(n),
                                                   dag_info,
                                                   defines),
                                                      obfuscation_map));

    fprintf(out, ")\n\n");
  }
  return 1;
}


/**Function********************************************************************

   Synopsis           [Writes the set of non-numeric constants as CONSTANTS
   statement]

   Description        [Returns 1 if at least one char have been written, 0
   otherwise]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_constants(const SymbTable_ptr symb_table, FILE* out)
{
  boolean written_once = false;
  SymbTableIter iter;

  SYMB_TABLE_FOREACH(symb_table, iter, STT_CONSTANT) {
    node_ptr name = SymbTable_iter_get_symbol(symb_table, &iter);

    if (node_get_type(name) == ATOM || node_get_type(name) == DOT) {
      /* a name to be written */
      if (!written_once) {
        fprintf(out, "CONSTANTS\n ");
        written_once = true;
      }
      else fprintf(out, ", ");

      print_node(out, name);
    }
  }

  if (written_once) {
    fprintf(out, " ;\n"); /* to close the statement */
    return 1;
  }

  return 0;
}


/**Function********************************************************************

   Synopsis           [Writes the set of non-numeric constants as CONSTANTS
   statement]

   Description        [Returns 1 if at least one char have been written, 0
   otherwise]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_write_obfuscated_constants(const SymbTable_ptr symb_table,
                                              FILE* out,
                                              hash_ptr obfuscation_map)
{
  boolean written_once = false;
  SymbTableIter iter;

  SYMB_TABLE_FOREACH(symb_table, iter, STT_CONSTANT) {
    node_ptr name = SymbTable_iter_get_symbol(symb_table, &iter);

    if (node_get_type(name) == ATOM || node_get_type(name) == DOT) {
      /* a name to be written */
      if (!written_once) {
        fprintf(out, "CONSTANTS\n ");
        written_once = true;
      }
      else fprintf(out, ", ");

      print_node(out, Compile_obfuscate_expression(symb_table,
                                                   name,
                                                   obfuscation_map));
    }
  }

  if (written_once) {
    fprintf(out, " ;\n"); /* to close the statement */
    return 1;
  }

  return 0;
}


/**Function********************************************************************

   Synopsis           [Generates the obfuscation map]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
hash_ptr Compile_get_obfuscation_map(const SymbTable_ptr symb_table) {
  hash_ptr res;
  /* WORD = 0, INTEGER = 1, ENUMERATIVE = 2, BOOLEAN = 3,
   * CONSTANT = 4, DEFINE = 5, ARRAY=6, ARRAY_DEFINES=7, WORDARRAY=8} */
  int counters[9] = {0,0,0,0,0,0,0,0,0};
  SymbTableIter iter;
  TypeChecker_ptr checker; /* Used to obtain the type of defines */

  checker = SymbTable_get_type_checker(symb_table);

  res = new_assoc();

  SYMB_TABLE_FOREACH(symb_table, iter, STT_VAR) {
    node_ptr var;
    SymbType_ptr type;
    char new_name[32];

    var = SymbTable_iter_get_symbol(symb_table, &iter);

    if (SymbTable_is_symbol_input_var(symb_table, var)) {
      new_name[0] = 'i';
    }
    else if (SymbTable_is_symbol_frozen_var(symb_table, var)) {
      new_name[0] = 'f';
    }
    else {
      new_name[0] = 's';
    }

    /* avoid giving new names to elements of array vars and to bits */
    if (BIT == node_get_type(var) ||
        is_array_var_element(symb_table, var)) {
        continue; /* get to the next var */
    }

    type = SymbTable_get_var_type(symb_table, var);

    if (SymbType_is_word(type)) {
      int size = 0;
      int chars;

      size = SymbType_get_word_width(type);

      chars = snprintf(&new_name[1], 31, "w%d_%d", size, counters[0]);
      SNPRINTF_CHECK(chars, 31);

      insert_assoc(res, var, find_node(ATOM,
                                       (node_ptr)find_string(new_name),
                                       Nil));
      counters[0]++;
    }
    else if (SymbType_is_boolean(type)) {
      int chars = snprintf(&new_name[1], 31, "b_%d", counters[3]);
      SNPRINTF_CHECK(chars, 31);

      insert_assoc(res, var, find_node(ATOM,
                                       (node_ptr)find_string(new_name),
                                       Nil));
      counters[3]++;
    }
    else if (SymbType_is_enum(type)) {
      int chars = snprintf(&new_name[1], 31, "s_%d", counters[2]);
      SNPRINTF_CHECK(chars, 31);

      insert_assoc(res, var, find_node(ATOM,
                                       (node_ptr)find_string(new_name),
                                       Nil));
      counters[2]++;
    }
    else if (SymbType_is_infinite_precision(type)) {
      int chars = snprintf(&new_name[1], 31, "i_%d", counters[1]);
      SNPRINTF_CHECK(chars, 31);

      insert_assoc(res, var, find_node(ATOM,
                                       (node_ptr)find_string(new_name),
                                       Nil));
      counters[1]++;
    }
    else if (SymbType_is_wordarray(type)) {
      int chars = snprintf(&new_name[1], 31, "wa_%d", counters[8]);
      SNPRINTF_CHECK(chars, 31);

      insert_assoc(res, var, find_node(ATOM,
                                       (node_ptr)find_string(new_name),
                                       Nil));
      counters[8]++;
    }

    else {
      rpterr("Unknown type for obfuscation.");
      error_unreachable_code();
    }
  }

  SYMB_TABLE_FOREACH(symb_table, iter, STT_CONSTANT) {
    node_ptr constant = SymbTable_iter_get_symbol(symb_table, &iter);
    char new_name[32];

    if ((node_get_type(constant) == ATOM) ||
        (node_get_type(constant) == DOT)) {
      int chars = snprintf(new_name, 32, "c_%d", counters[4]);
      SNPRINTF_CHECK(chars, 32);

      insert_assoc(res, constant, find_node(ATOM,
                                            (node_ptr)find_string(new_name),
                                            Nil));
      counters[4]++;
    }
  }

  SYMB_TABLE_FOREACH(symb_table, iter, STT_VARIABLE_ARRAY) {
    node_ptr st = SymbTable_iter_get_symbol(symb_table, &iter);
    char new_name[32];
    int chars;

    /* avoid giving new names to elements of array vars */
    if (is_array_var_element(symb_table, st)) {
        continue; /* get to the next var */
    }

    chars = snprintf(new_name, 32, "a%d", counters[6]);
    SNPRINTF_CHECK(chars, 32);

    insert_assoc(res, st, find_node(ATOM,
                                    (node_ptr)find_string(new_name),
                                    Nil));
    counters[6]++;
  }

  SYMB_TABLE_FOREACH(symb_table, iter, STT_DEFINE) {
    node_ptr def = SymbTable_iter_get_symbol(symb_table, &iter);
    char new_name[32];
    SymbType_ptr dtype;
    char *t;
    int size = 0;

    /* avoid giving new names to elements of array defines */
    if (is_array_define_element(symb_table, def)) {
        continue; /* get to the next var */
    }

    dtype = TypeChecker_get_expression_type(checker,
                          SymbTable_get_define_body(symb_table, def),
                          SymbTable_get_define_context(symb_table, def));

    if (SymbType_is_word(dtype)) {
      size = SymbType_get_word_width(dtype);
      t = "w";
    }
    else if (SymbType_is_boolean(dtype)) {
      t = "b";
    }
    else if (SymbType_is_enum(dtype)) {
      t = "s";
    }
    else if (SymbType_is_infinite_precision(dtype)) {
      t = "i";
    }
    else if (SymbType_is_wordarray(dtype)) {
      t = "wa";
    }
    else {
      /* Array */
      t = "a";
    }

    if (!strcmp("w", t)) { /* Is a word type */
      /* Print also the word size */
      int chars = snprintf(new_name, 32, "dw%d_%d", size, counters[5]);
      SNPRINTF_CHECK(chars, 32);
    }
    else {
      int chars = snprintf(new_name, 32, "d%s_%d", t, counters[5]);
      SNPRINTF_CHECK(chars, 32);
    }
    insert_assoc(res, def, find_node(ATOM,
                                     (node_ptr)find_string(new_name),
                                     Nil));
    counters[5]++;
  }

  SYMB_TABLE_FOREACH(symb_table, iter, STT_ARRAY_DEFINE) {
    node_ptr def = SymbTable_iter_get_symbol(symb_table, &iter);
    char new_name[32];
    int chars;

    /* avoid giving new names to elements of array defines */
    if (is_array_define_element(symb_table, def)) {
        continue; /* get to the next var */
    }

    chars = snprintf(new_name, 32, "m%d", counters[7]);
    SNPRINTF_CHECK(chars, 32);

    insert_assoc(res, def, find_node(ATOM,
                                     (node_ptr)find_string(new_name),
                                     Nil));
    counters[7]++;
  }

  return res;
}


/**Function********************************************************************

   Synopsis           [Prints the obfuscated flatten version of FSM of an
   SMV model.]

   Description        []

   SideEffects        []

   SeeAlso            [compile_write_flat_fsm]

******************************************************************************/
static void compile_write_obfuscated_flat_fsm(FILE* out,
                                              const SymbTable_ptr symb_table,
                                              const array_t* layer_names,
                                              const char* fsm_name,
                                              FlatHierarchy_ptr hierarchy,
                                              hash_ptr dag_info,
                                              hash_ptr defines,
                                              hash_ptr obfuscation_map,
                                              boolean force_flattening,
                                              hash_ptr cdh)
{
  int i;
  const char* name;
  char* ifile = get_input_file(OptsHandler_get_instance());

  nusmv_assert(layer_names != (array_t*) NULL);

  fprintf(out, "-- Obfuscated and flattened FSM model generated from %s\n"
          "-- Dumped layers are: ", ((char*)NULL != ifile ? ifile : "stdin"));

  /* dumps the layer names: */
  arrayForEachItem(const char*, layer_names, i, name) {
    fprintf(out, "%s ", name);
  }
  fprintf(out, "\n\n");

  fprintf(out, "%s\n", fsm_name);

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Input variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      if (SymbLayer_get_input_vars_num(layer) > 0) {
        compile_write_obfuscated_flatten_vars(symb_table, out,
                                              layer, STT_INPUT_VAR,
                                              obfuscation_map);
      }
    }
  }

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- State variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      if (SymbLayer_get_state_vars_num(layer) > 0) {
        compile_write_obfuscated_flatten_vars(symb_table, out,
                                              layer, STT_STATE_VAR,
                                              obfuscation_map);
      }
    }
  }

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Frozen variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      if (SymbLayer_get_frozen_vars_num(layer) > 0) {
        compile_write_obfuscated_flatten_vars(symb_table, out,
                                              layer, STT_FROZEN_VAR,
                                              obfuscation_map);
      }
    }
  }

  /* NOTE that array variables (i.e. variable_array) are not output as
     they are output during vars outputting in
     compile_write_flatten_vars */

  /* DEFINEs */
  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Defines from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      compile_write_obfuscated_flat_define(symb_table,
                                           out,
                                           layer,
                                           dag_info,
                                           defines,
                                           obfuscation_map,
                                           force_flattening);
    }
  }

  /* NOTE that array defines (i.e. array_define) are not output as
     they are output during normal defines outputting in
     compile_write_obfuscated_flat_define  */

  /* CONSTANTS */
  if (!opt_backward_comp(OptsHandler_get_instance())) {
    if (compile_write_obfuscated_constants(symb_table,
                                           out, obfuscation_map)) {
      fprintf(out, "\n");
    }
  }

  /* ASSIGNs */
  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Assignments from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      compile_write_obfuscated_flat_asgn(symb_table,
                                         out,
                                         layer,
                                         hierarchy,
                                         dag_info,
                                         defines,
                                         obfuscation_map,
                                         cdh);
    }
  }

  /* CONSTRAINS (except assigns) */
  if (compile_write_obfuscated_flatten_expr_split(symb_table,
                                                  out,
                                                  FlatHierarchy_get_init(hierarchy),
                                                  "INIT\n",
                                                  dag_info,
                                                  defines,
                                                  obfuscation_map,
                                                  force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  if (compile_write_obfuscated_flatten_expr_split(symb_table,
                                                  out,
                                                  FlatHierarchy_get_invar(hierarchy),
                                                  "INVAR\n",
                                                  dag_info,
                                                  defines,
                                                  obfuscation_map,
                                                  force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  if (compile_write_obfuscated_flatten_expr_split(symb_table,
                                                  out,
                                                  FlatHierarchy_get_trans(hierarchy),
                                                  "TRANS\n",
                                                  dag_info,
                                                  defines,
                                                  obfuscation_map,
                                                  force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  {
    node_ptr fc = FlatHierarchy_get_justice(hierarchy);
    boolean are_there_compassion =
      (Nil != FlatHierarchy_get_compassion(hierarchy));

    while (Nil != fc) {
      if (compile_write_obfuscated_flatten_expr(symb_table, out, car(fc),
                                                /* For backward compatibility */
                                                are_there_compassion ?
                                                "JUSTICE\n" : "FAIRNESS\n",
                                                dag_info, defines,
                                                obfuscation_map,
                                                force_flattening, cdh)) {
        fprintf(out, "\n");
      }
      fc = cdr(fc);
    }
  }

  if (compile_write_obfuscated_flatten_expr_pair(symb_table,
                                                 out,
                                                 FlatHierarchy_get_compassion(hierarchy),
                                                 "COMPASSION\n",
                                                 dag_info,
                                                 defines,
                                                 obfuscation_map,
                                                 force_flattening, cdh)) {
    fprintf(out, "\n");
  }
  return ;
}


/**Function********************************************************************

   Synopsis           [Prints the flatten version of FSM of an SMV model.]

   Description        [Prints on the specified file the flatten
   FSM of an SMV model, i.e. a list of all variable, defines, and all
   constrains (INIT, TRANS, INVAR, ASSIGNS, JUSTICE, COMPASSION).
   Specifications are NOT printed.

   layer_names is an array of names of layers that is typically
   obtained from the symbol table. fsm_name is a name of the output
   structure, usually it is "MODULE main".  ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_flat_fsm(FILE* out,
                                   const SymbTable_ptr symb_table,
                                   const array_t* layer_names,
                                   const char* fsm_name,
                                   FlatHierarchy_ptr hierarchy,
                                   hash_ptr dag_info, hash_ptr defines,
                                   boolean force_flattening,
                                   hash_ptr cdh)
{
  SymbLayerIter iter;
  NodeList_ptr tmp;
  int i;
  const char* name;
  char* ifile = get_input_file(OptsHandler_get_instance());

  nusmv_assert(layer_names != (array_t*) NULL);

  fprintf(out, "-- Flattened FSM model generated from %s\n"
          "-- Dumped layers are: ", ((char*)NULL != ifile ? ifile : "stdin"));

  /* dumps the layer names: */
  arrayForEachItem(const char*, layer_names, i, name) {
    fprintf(out, "%s ", name);
  }
  fprintf(out, "\n\n");

  fprintf(out, "%s\n", fsm_name);

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Input variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      SymbLayer_gen_iter(layer, &iter, STT_INPUT_VAR);
      compile_write_flatten_vars(symb_table, out, layer, &iter);
    }
  }

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- State variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      SymbLayer_gen_iter(layer, &iter, STT_STATE_VAR);
      compile_write_flatten_vars(symb_table, out, layer, &iter);
    }
  }

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Frozen variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      SymbLayer_gen_iter(layer, &iter, STT_FROZEN_VAR);
      compile_write_flatten_vars(symb_table, out, layer, &iter);
    }
  }

  /* NOTE that array variables (i.e. variable_array) are not output as
     they are output during vars outputting in
     compile_write_flatten_vars */

  /* DEFINEs */
  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    NodeList_ptr defines_list;

    SymbLayer_gen_iter(layer, &iter, STT_DEFINE);
    defines_list = SymbLayer_iter_to_list(layer, iter);

    fprintf(out, "-- Defines from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      compile_write_flat_define(symb_table,
                                out,
                                defines_list,
                                dag_info,
                                defines,
                                force_flattening);
    }

    NodeList_destroy(defines_list);
  }

  /* NOTE that array defines (i.e. array_define) are not output as
     they are output during normal defines outputting in
     compile_write_flat_define  */

  /* CONSTANTS */
  if (!opt_backward_comp(OptsHandler_get_instance())) {
    if (compile_write_constants(symb_table, out)) fprintf(out, "\n");
  }

  /* ASSIGNs */
  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Assignments from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      SymbLayer_gen_iter(layer, &iter, STT_VAR);
      tmp = SymbLayer_iter_to_list(layer, iter);

      compile_write_flat_asgn(symb_table, out, tmp,
                              hierarchy, dag_info, defines, cdh);
      NodeList_destroy(tmp);
    }
  }

  /* CONSTRAINS (except assigns) */
  if (compile_write_flatten_expr_split(symb_table,
                                       out,
                                       FlatHierarchy_get_init(hierarchy),
                                       "INIT\n",
                                       dag_info,
                                       defines,
                                       force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  if (compile_write_flatten_expr_split(symb_table, out,
                                       FlatHierarchy_get_invar(hierarchy),
                                       "INVAR\n", dag_info, defines,
                                       force_flattening, cdh)) {
    fprintf(out, "\n");
  }
  if (compile_write_flatten_expr_split(symb_table, out,
                                       FlatHierarchy_get_trans(hierarchy),
                                       "TRANS\n", dag_info, defines,
                                       force_flattening, cdh)) {
    fprintf(out, "\n");
  }
  {
    node_ptr fc = FlatHierarchy_get_justice(hierarchy);
    boolean are_there_compassion =
      (Nil != FlatHierarchy_get_compassion(hierarchy));

    while (Nil != fc) {
      if (compile_write_flatten_expr(symb_table, out, car(fc),
                                     /* For backward compatibility */
                                     are_there_compassion ?
                                     "JUSTICE\n" : "FAIRNESS\n",
                                     dag_info, defines,
                                     force_flattening, cdh)) {
        fprintf(out, "\n");
      }
      fc = cdr(fc);
    }
  }

  if (compile_write_flatten_expr_pair(symb_table,
                                      out,
                                      FlatHierarchy_get_compassion(hierarchy),
                                      "COMPASSION\n",
                                      dag_info,
                                      defines,
                                      force_flattening,
                                      cdh)) {
    fprintf(out, "\n");
  }
  return ;
}

/**Function********************************************************************

   Synopsis           [Filter for a SymbLayer iter, filters out variables
                       that are not in the set given as argument]

   Description        [Filter for a SymbLayer iter, filters out variables
                       that are not in the set given as argument]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean
compile_write_is_var_in_set(const SymbLayer_ptr layer,
                            const node_ptr sym, void* arg)
{
  Set_t keep_vars = (Set_t)arg;

  return Set_IsMember(keep_vars, sym);
}


/**Function********************************************************************

   Synopsis           [Prints the restricted flatten version of FSM of
                       an SMV model.]

   Description        [Prints on the specified file the flatten
   FSM of an SMV model, i.e. a list of restricted variables, defines, and all
   constrains (INIT, TRANS, INVAR, ASSIGNS, JUSTICE, COMPASSION) restricted to
   the set of variables in the FlatHierarchy.
   Specifications are NOT printed.

   layer_names is an array of names of layers that is typically
   obtained from the symbol table. fsm_name is a name of the output
   structure, usually it is "MODULE main".  ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_restricted_flat_fsm(FILE* out,
                                              const SymbTable_ptr symb_table,
                                              const array_t* layer_names,
                                              const char* fsm_name,
                                              FlatHierarchy_ptr hierarchy,
                                              hash_ptr dag_info,
                                              hash_ptr defines,
                                              boolean force_flattening,
                                              hash_ptr cdh)
{
  Set_t restrict_on = FlatHierarchy_get_vars(hierarchy);

  int i;
  const char* name;
  char* ifile = get_input_file(OptsHandler_get_instance());

  nusmv_assert(layer_names != (array_t*) NULL);

  fprintf(out, "-- Flattened FSM model generated from %s\n"
          "-- Dumped layers are: ", ((char*)NULL != ifile ? ifile : "stdin"));

  /* dumps the layer names: */
  arrayForEachItem(const char*, layer_names, i, name) {
    fprintf(out, "%s ", name);
  }
  fprintf(out, "\n\n");

  fprintf(out, "%s\n", fsm_name);

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Input variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      SymbLayerIter iter;

      SymbLayer_gen_iter(layer, &iter, STT_INPUT_VAR);
      SymbLayer_iter_set_filter(layer, &iter,
                                compile_write_is_var_in_set, restrict_on);
      compile_write_flatten_vars(symb_table, out, layer, &iter);
    }
  }

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- State variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      SymbLayerIter iter;

      SymbLayer_gen_iter(layer, &iter, STT_STATE_VAR);
      SymbLayer_iter_set_filter(layer, &iter,
                                compile_write_is_var_in_set, restrict_on);
      compile_write_flatten_vars(symb_table, out, layer, &iter);
    }
  }

  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Frozen variables from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      SymbLayerIter iter;

      SymbLayer_gen_iter(layer, &iter, STT_FROZEN_VAR);
      SymbLayer_iter_set_filter(layer, &iter,
                                compile_write_is_var_in_set, restrict_on);
      compile_write_flatten_vars(symb_table, out, layer, &iter);
    }
  }

  /* NOTE that array variables (i.e. variable_array) are not output as
     they are output during vars outputting in
     compile_write_flatten_vars */

  {
    /* DEFINEs */
    arrayForEachItem(const char*, layer_names, i, name) {
      SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);

      fprintf(out, "-- Defines from layer '%s'\n", name);
      if (layer != SYMB_LAYER(NULL)) {
        /* We want to add only defines that actually only contain
           variables that are in the COI. Otherwise, we may have
           expressions refering to undeclare variables */

        NodeList_ptr restricted_defines = NodeList_create();
        SymbLayerIter iter;

        SYMB_LAYER_FOREACH(layer, iter, STT_DEFINE) {
          node_ptr define = SymbLayer_iter_get_symbol(layer, &iter);

          node_ptr body = SymbTable_get_define_body(symb_table,
                                                    define);
          node_ptr ctx = SymbTable_get_define_context(symb_table,
                                                      define);

          Set_t deps =  Formula_GetDependencies(symb_table, body, ctx);

          if (Set_Contains(restrict_on, deps)) {
            NodeList_append(restricted_defines, define);
          }
        }

        compile_write_flat_define(symb_table,
                                  out,
                                  restricted_defines,
                                  dag_info,
                                  defines,
                                  force_flattening);

      NodeList_destroy(restricted_defines);
      }
    }
  }

  /* NOTE that array defines (i.e. array_define) are not output as
     they are output during normal defines outputting in
     compile_write_flat_define  */

  /* CONSTANTS */
  if (!opt_backward_comp(OptsHandler_get_instance())) {
    if (compile_write_constants(symb_table, out)) fprintf(out, "\n");
  }

  /* ASSIGNs */
  arrayForEachItem(const char*, layer_names, i, name) {
    SymbLayer_ptr layer = SymbTable_get_layer(symb_table, name);
    fprintf(out, "-- Assignments from layer '%s'\n", name);
    if (layer != SYMB_LAYER(NULL)) {
      SymbLayerIter iter;
      Set_t layer_vars;
      NodeList_ptr tmp;

      SymbLayer_gen_iter(layer, &iter, STT_VAR);
      layer_vars = SymbLayer_iter_to_set(layer, iter);

      layer_vars = Set_Intersection(layer_vars, restrict_on);

      tmp  = Set_Set2List(layer_vars);
      compile_write_flat_asgn(symb_table, out, tmp,
                              hierarchy, dag_info, defines, cdh);
      Set_ReleaseSet(layer_vars);
    }
  }

  /* CONSTRAINS (except assigns) */
  if (compile_write_flatten_expr_split(symb_table,
                                       out,
                                       FlatHierarchy_get_init(hierarchy),
                                       "INIT\n",
                                       dag_info,
                                       defines, force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  if (compile_write_flatten_expr_split(symb_table,
                                       out,
                                       FlatHierarchy_get_invar(hierarchy),
                                       "INVAR\n",
                                       dag_info,
                                       defines, force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  if (compile_write_flatten_expr_split(symb_table,
                                       out,
                                       FlatHierarchy_get_trans(hierarchy),
                                       "TRANS\n",
                                       dag_info,
                                       defines, force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  {
    node_ptr fc = FlatHierarchy_get_justice(hierarchy);
    boolean are_there_compassion =
      (Nil != FlatHierarchy_get_compassion(hierarchy));

    while (Nil != fc) {
      if (compile_write_flatten_expr(symb_table, out, car(fc),
                                     /* For backward compatibility */
                                     are_there_compassion ?
                                     "JUSTICE\n" : "FAIRNESS\n",
                                     dag_info, defines,
                                     force_flattening,
                                     cdh)) {
        fprintf(out, "\n");
      }
      fc = cdr(fc);
    }
  }

  if (compile_write_flatten_expr_pair(symb_table,
                                      out,
                                      FlatHierarchy_get_compassion(hierarchy),
                                      "COMPASSION\n",
                                      dag_info,
                                      defines,
                                      force_flattening,
                                      cdh)) {
    fprintf(out, "\n");
  }
  return ;
}


/**Function********************************************************************

   Synopsis           [Prints the given flatten specifications.]

   Description        [Prints into the specified file the flatten
   specifications.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_flat_spec(FILE* out, const SymbTable_ptr symb_table,
                                    node_ptr spec, const char* msg,
                                    hash_ptr dag_info, hash_ptr defines,
                                    boolean force_flattening,
                                    hash_ptr cdh)
{
  if (compile_write_flatten_spec_split(symb_table, out, spec, msg,
                                       dag_info, defines,
                                       force_flattening, cdh)) {
    fprintf(out, "\n");
  }
}

/**Function********************************************************************

   Synopsis           [Prints the given flatten specifications.]

   Description        [Prints into the specified file the flatten
   specifications.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_obfuscated_flat_spec(FILE* out,
                                               const SymbTable_ptr symb_table,
                                               node_ptr spec,
                                               const char* msg,
                                               hash_ptr dag_info,
                                               hash_ptr defines,
                                               hash_ptr obfuscation_map,
                                               boolean force_flattening,
                                               hash_ptr cdh)
{
  if (compile_write_obfuscated_flatten_spec_split(symb_table,
                                                  out,
                                                  spec,
                                                  msg,
                                                  dag_info,
                                                  defines,
                                                  obfuscation_map,
                                                  force_flattening,
                                                  cdh)) {
    fprintf(out, "\n");
  }
}




/**Function********************************************************************

   Synopsis           [Prints the flatten specifications of an SMV model.]

   Description        [Prints into the specified file the
   specifications of an SMV model.
   ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_flat_specs(FILE* out,
                                     const SymbTable_ptr st,
                                     FlatHierarchy_ptr hierarchy,
                                     hash_ptr dag_info, hash_ptr defines,
                                     boolean force_flattening,
                                     hash_ptr cdh)
{
  /* dumps the properties */
  compile_write_flat_spec(out, st, FlatHierarchy_get_spec(hierarchy),
                          "CTLSPEC\n", dag_info, defines, force_flattening,
                          cdh);
  compile_write_flat_spec(out, st, FlatHierarchy_get_compute(hierarchy),
                          "COMPUTE\n", dag_info, defines, force_flattening,
                          cdh);
  compile_write_flat_spec(out, st, FlatHierarchy_get_ltlspec(hierarchy),
                          "LTLSPEC\n", dag_info, defines, force_flattening,
                          cdh);
  compile_write_flat_spec(out, st, FlatHierarchy_get_invarspec(hierarchy),
                          "INVARSPEC\n", dag_info, defines, force_flattening,
                          cdh);

  { /* PSL specifications are not supported at the moment */
    node_ptr pslspec = FlatHierarchy_get_pslspec(hierarchy);
    if (pslspec != Nil) {
      fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
      fprintf(nusmv_stderr,
              "This version does not support the flattening of PSL properties.\n"
              "However, for user's convenience all the PSL properties will be dumped\n"
              "as comments in the output file.\n");
      fprintf(nusmv_stderr, "******** END WARNING ********\n\n");

      fprintf(out,
              "--- Dumping of PSL properties is not supported by this version of the system.\n"\
              "--- However, the PSL properties had been dumped here for user's convenience,\n"\
              "--- as the occurred in the original model. \n");
      compile_write_flatten_psl(st, out, pslspec, dag_info, defines, cdh);
    }
  }
}


/**Function********************************************************************

   Synopsis           [Prints the obfuscated flatten specifications of an
   SMV model.]

   Description        []

   SideEffects        []

   SeeAlso            [compile_write_flat_specs]

******************************************************************************/
static void compile_write_obfuscated_flat_specs(FILE* out,
                                                const SymbTable_ptr st,
                                                FlatHierarchy_ptr hierarchy,
                                                hash_ptr dag_info,
                                                hash_ptr defines,
                                                hash_ptr obfuscation_map,
                                                boolean force_flattening,
                                                hash_ptr cdh)
{
  /* dumps the properties */
  compile_write_obfuscated_flat_spec(out,
                                     st,
                                     FlatHierarchy_get_spec(hierarchy),
                                     "CTLSPEC\n",
                                     dag_info,
                                     defines,
                                     obfuscation_map,
                                     force_flattening,
                                     cdh);
  compile_write_obfuscated_flat_spec(out,
                                     st,
                                     FlatHierarchy_get_compute(hierarchy),
                                     "COMPUTE\n",
                                     dag_info,
                                     defines,
                                     obfuscation_map,
                                     force_flattening,
                                     cdh);
  compile_write_obfuscated_flat_spec(out,
                                     st,
                                     FlatHierarchy_get_ltlspec(hierarchy),
                                     "LTLSPEC\n",
                                     dag_info,
                                     defines,
                                     obfuscation_map,
                                     force_flattening,
                                     cdh);
  compile_write_obfuscated_flat_spec(out,
                                     st,
                                     FlatHierarchy_get_invarspec(hierarchy),
                                     "INVARSPEC\n",
                                     dag_info,
                                     defines,
                                     obfuscation_map,
                                     force_flattening,
                                     cdh);

  { /* PSL specifications are not supported at the moment */
    node_ptr pslspec = FlatHierarchy_get_pslspec(hierarchy);
    if (pslspec != Nil) {
      fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
      fprintf(nusmv_stderr,
              "This version does not support the flattening of PSL properties.\n"
              "However, for user's convenience all the PSL properties will be dumped\n"
              "as comments in the output file.\n");
      fprintf(nusmv_stderr, "******** END WARNING ********\n\n");

      fprintf(out,
              "--- Dumping of PSL properties is not supported by this version of the system.\n"\
              "--- However, the PSL properties had been dumped here for user's convenience,\n"\
              "--- as the occurred in the original model. \n");
      compile_write_flatten_psl(st, out, pslspec, dag_info, defines, cdh);
    }
  }
}



/**Function********************************************************************

   Synopsis           [Prints the boolean FSM of an SMV model.]

   Description        [Prints into the specified file the boolean FSM of an
   SMV model.
   bool_sexp_fsm should be a boolean Sexp FSM.
   layer_names is an array of layers whose variables will be printed,
   usually this parameter is a list of all layers committed to enc. The array
   should be ended by a NULL element.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_bool_fsm(FILE* out,
                                   const SymbTable_ptr symb_table,
                                   NodeList_ptr layers,
                                   const char* fsm_name,
                                   BoolSexpFsm_ptr bool_sexp_fsm,
                                   hash_ptr dag_info, hash_ptr defines,
                                   boolean force_flattening,
                                   hash_ptr cdh)
{
  BoolEnc_ptr benc;
  ListIter_ptr iter;
  char* ifile = get_input_file(OptsHandler_get_instance());
  /* must have been booleanized */
  nusmv_assert(SexpFsm_is_boolean(SEXP_FSM(bool_sexp_fsm)));

  benc = BoolSexpFsm_get_bool_enc(bool_sexp_fsm);

  fprintf(out,
          "-- Flattened Boolean FSM model  generated from %s\n"
          "-- Dumped layers are: ",
          ((char*)NULL != ifile ? ifile : "stdin"));

  /* dumps the layer names: */
  iter = NodeList_get_first_iter(layers);
  while (!ListIter_is_end(iter)) {
    SymbLayer_ptr layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
    fprintf(out, "%s", SymbLayer_get_name(layer));
    fprintf(out, " ");
    iter = ListIter_get_next(iter);
  }
  fprintf(out, "\n\n");

  fprintf(out, "%s\n", fsm_name);

  /* NOTE: Defines are not dumped, therefore should not be booleanized */

  /* iter = NodeList_get_first_iter(layers); */
  /* while (!ListIter_is_end(iter)) { */
  /*   SymbLayer_ptr layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter)); */
  /*   NodeList_ptr defines_list; */
  /*   SymbLayerIter sliter; */

  /*   SymbLayer_gen_iter(layer, &sliter, STT_DEFINE); */
  /*   defines_list = SymbLayer_iter_to_list(layer, sliter); */

  /*   fprintf(out, "-- Defines from layer '%s'\n", SymbLayer_get_name(layer)); */
  /*   if (layer != SYMB_LAYER(NULL)) { */
  /*     compile_write_flat_define(symb_table, */
  /*                               out, */
  /*                               defines_list, */
  /*                               dag_info, */
  /*                               defines, */
  /*                               force_flattening); */
  /*   } */

  /*   NodeList_destroy(defines_list); */
  /*   iter = ListIter_get_next(iter); */
  /* } */

  /* Input variables */
  iter = NodeList_get_first_iter(layers);
  while (!ListIter_is_end(iter)) {
    SymbLayer_ptr layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
    fprintf(out, "-- Input variables from layer '%s'\n",
            SymbLayer_get_name(layer));
    if (SymbLayer_get_input_vars_num(layer) > 0) {
      compile_write_flatten_bool_vars(symb_table, benc,
                                      out, layer, STT_INPUT_VAR);
    }
    iter = ListIter_get_next(iter);
  }

  /* State variables */
  iter = NodeList_get_first_iter(layers);
  while (!ListIter_is_end(iter)) {
    SymbLayer_ptr layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
    fprintf(out, "-- State variables from layer '%s'\n",
            SymbLayer_get_name(layer));
    if (SymbLayer_get_state_vars_num(layer) > 0) {
      compile_write_flatten_bool_vars(symb_table, benc,
                                      out, layer, STT_STATE_VAR);
    }
    iter = ListIter_get_next(iter);
  }

  /* Frozen variables */
  iter = NodeList_get_first_iter(layers);
  while (!ListIter_is_end(iter)) {
    SymbLayer_ptr layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
    fprintf(out, "-- Frozen variables from layer '%s'\n",
            SymbLayer_get_name(layer));
    if (SymbLayer_get_frozen_vars_num(layer) > 0) {
      compile_write_flatten_bool_vars(symb_table, benc,
                                      out, layer, STT_FROZEN_VAR);
    }
    iter = ListIter_get_next(iter);
  }

  /* CONSTANTS */
  if (!opt_backward_comp(OptsHandler_get_instance())) {
    if (compile_write_constants(symb_table, out)) fprintf(out, "\n");
  }

  /* CONSTRAINS */
  if (compile_write_flatten_expr_split(symb_table, out,
                                       SexpFsm_get_init(SEXP_FSM(bool_sexp_fsm)),
                                       "INIT\n", dag_info, defines,
                                       force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  if (compile_write_flatten_expr_split(symb_table, out,
                                       SexpFsm_get_invar(SEXP_FSM(bool_sexp_fsm)),
                                       "INVAR\n", dag_info, defines,
                                       force_flattening, cdh)) {
    fprintf(out, "\n");
  }

  if (compile_write_flatten_expr_split(symb_table, out,
                                       SexpFsm_get_trans(SEXP_FSM(bool_sexp_fsm)),
                                       "TRANS\n", dag_info, defines,
                                       force_flattening, cdh)) {
    fprintf(out, "\n");
  }


  {
    node_ptr fc = SexpFsm_get_justice(SEXP_FSM(bool_sexp_fsm));
    boolean are_there_compassion =
      (Nil != SexpFsm_get_compassion(SEXP_FSM(bool_sexp_fsm)));

    while(Nil != fc) {
      if (compile_write_flatten_expr(symb_table, out,
                                     car(fc),
                                     /* For backward compatibility */
                                     are_there_compassion ?
                                     "JUSTICE\n" : "FAIRNESS\n",
                                     dag_info, defines,
                                     force_flattening, cdh)) {
        fprintf(out, "\n");
      }
      fc = cdr(fc);
    }
  }

  if (compile_write_flatten_expr_pair(symb_table, out,
                                      SexpFsm_get_compassion(SEXP_FSM(bool_sexp_fsm)),
                                      "COMPASSION\n", dag_info, defines,
                                      force_flattening, cdh)) {
    fprintf(out, "\n\n");
  }

}


/**Function********************************************************************

   Synopsis           [Private service to print a boolean specification]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_bool_spec(FILE* out, BddEnc_ptr enc,
                                    node_ptr spec, const char* msg,
                                    SymbLayer_ptr det_layer,
                                    hash_ptr dag_info, hash_ptr defines,
                                    hash_ptr cdh)
{
  if (compile_write_flatten_bfexpr(enc, BaseEnc_get_symb_table(BASE_ENC(enc)),
                                   det_layer, out, spec,
                                   msg, dag_info, defines, cdh)) {
    fprintf(out, "\n");
  }
}


/**Function********************************************************************

   Synopsis           [Prints the boolean specifications of an SMV model.]

   Description        [Prints into the specified file the booleanized
   specifications of an SMV model.

   NOTE: a temporary layer will be created during the dumping for
   determinization variables that derived from the booleanization of
   the specifications. These variable declarations will be printed
   after the specs.
   ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_bool_specs(FILE* out,
                                     BddEnc_ptr enc,
                                     SymbLayer_ptr det_layer,
                                     FlatHierarchy_ptr hierarchy,
                                     hash_ptr dag_info,
                                     hash_ptr defines,
                                     hash_ptr cdh)
{
  /* here we create a temporary layer, in order to hold all
     determinization variables that will be created by the
     booleanization of the properties. This layer will be destroyed
     after the printing of the determinization variables that it will
     possibly contain */
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));

  /* dumps the properties */
  compile_write_bool_spec(out, enc,
                          FlatHierarchy_get_spec(hierarchy),
                          "CTLSPEC\n", det_layer,
                          dag_info, defines, cdh);
  compile_write_bool_spec(out, enc, FlatHierarchy_get_compute(hierarchy),
                          "COMPUTE\n", det_layer,
                          dag_info, defines, cdh);
  compile_write_bool_spec(out, enc, FlatHierarchy_get_ltlspec(hierarchy),
                          "LTLSPEC\n", det_layer,
                          dag_info, defines, cdh);
  compile_write_bool_spec(out, enc, FlatHierarchy_get_invarspec(hierarchy),
                          "INVARSPEC\n", det_layer,
                          dag_info, defines, cdh);

  { /* PSL specifications are not supported at the moment */
    node_ptr pslspec = FlatHierarchy_get_pslspec(hierarchy);
    if (pslspec != Nil) {
      fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
      fprintf(nusmv_stderr,
              "This version does not support the booleanization of PSL properties.\n"
              "However, for user's convenience all the PSL properties will be dumped\n"
              "as comments in the output file.\n");
      fprintf(nusmv_stderr, "******** END WARNING ********\n\n");

      fprintf(out,
              "--- Dumping of PSL properties is not supported by this version of the system.\n"\
              "--- However, the PSL properties had been dumped here for user's convenience,\n"\
              "--- as the occurred in the original model. \n");
      compile_write_flatten_psl(st, out, pslspec, dag_info, defines, cdh);
    }
  }

  /* Dumping of the determinization vars,
     and destruction of the temporary layer */
  if (SymbLayer_get_input_vars_num(det_layer) > 0) {
    fprintf(out, "-- Determinization variables of specifications:\n");
    compile_write_flatten_bool_vars(st,
                                    BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(enc)),
                                    out, det_layer, STT_INPUT_VAR);
  }
}

/**Function********************************************************************

   Synopsis           []

   Description        [Private service of function Compile_convert_to_dag]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr compile_convert_to_dag_aux(SymbTable_ptr symb_table,
                                           node_ptr expr, hash_ptr hash,
                                           unsigned int num_thres,
                                           unsigned int dep_thres,
                                           hash_ptr defines,
                                           const char* defines_prefix)
{
  node_ptr info;
  node_ptr define;

  if (expr == Nil) return Nil;

  /* We ignore defining sysmbols for leaves */
  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case TRUEEXP: case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD: case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC: case NUMBER_REAL: case NUMBER_EXP:
  case UWCONST: case SWCONST:
  case ATOM: case BIT: case ARRAY: case SELF: case DOT:
    return expr;
  default:
    /* This is intentionally empty */
    break;
  }

  if (defines != (hash_ptr) NULL) {
    define = find_assoc(defines, expr);
    if (define != Nil) {
      /* found a previously inserted define that substitutes the expression */
      unsigned int count;
      nusmv_assert(node_get_type(define) == COLON);

      /* Increment hit counter */
      INCREMENT_HITS_NUMBER();

      /* this counter keeps track of really used defines, for later dumping.
         setcdr can be used here as the node was created with new_node  */
      count = PTR_TO_INT(cdr(define));

      setcdr(define, PTR_FROM_INT(node_ptr, count+1));
      nusmv_assert(node_get_type(car(define)) == EQDEF);
      return car(car(define)); /* returns the name */
    }
  }

  if (hash != (hash_ptr) NULL) {
    nusmv_assert(defines != (hash_ptr) NULL);

    info = find_assoc(hash, expr);
    if (info != Nil) {
      unsigned int count;
      unsigned int depth;
      boolean admissible;

      /* found a node that might be substituted if needed */
      compile_unpack_dag_info(info, &count, &depth, &admissible);
      if (admissible &&
          (count >= num_thres || (count > 1 && depth >= dep_thres))) {
        /* simplifies to a new dag node: continue on children */
        node_ptr name;
        node_ptr left = compile_convert_to_dag_aux(symb_table,
                                                   car(expr), hash,
                                                   num_thres, dep_thres,
                                                   defines, defines_prefix);
        node_ptr right = compile_convert_to_dag_aux(symb_table,
                                                    cdr(expr), hash,
                                                    num_thres, dep_thres,
                                                    defines, defines_prefix);
        node_ptr body = find_node(node_get_type(expr), left, right);

        name = __create_define_name(symb_table, defines_prefix, body);

        define = new_node(COLON, new_node(EQDEF, name, body),
                          PTR_FROM_INT(node_ptr, 1));
        insert_assoc(defines, expr, define);

        return name;
      }
    }
  }

  /* no substitution found or needed */
  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case TRUEEXP: case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD: case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC: case NUMBER_REAL: case NUMBER_EXP:
  case UWCONST: case SWCONST:
  case ATOM: case BIT: case ARRAY: case SELF: case DOT:
    return expr;

  default:
    {
      node_ptr left = compile_convert_to_dag_aux(symb_table, car(expr), hash,
                                                 num_thres, dep_thres,
                                                 defines, defines_prefix);
      node_ptr right = compile_convert_to_dag_aux(symb_table, cdr(expr), hash,
                                                  num_thres, dep_thres,
                                                  defines, defines_prefix);
      return find_node(node_get_type(expr), left, right);
    }
  }
}


/**Function********************************************************************

   Synopsis           []

   Description        [If det_layer is not NULL, then hierarchy is
   to be considered boolean, and specifications will be booleanized,
   If det_layer is null, then also enc can be null]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static hash_ptr
compile_create_dag_info_from_hierarchy(SymbTable_ptr st,
                                       FlatHierarchy_ptr hierarchy,
                                       SymbLayer_ptr det_layer,
                                       BddEnc_ptr enc,
                                       boolean force_flattening,
                                       hash_ptr cdh)
{
  hash_ptr dag_info = new_assoc();
  node_ptr specs[] = { FlatHierarchy_get_spec(hierarchy),
                       FlatHierarchy_get_compute(hierarchy),
                       FlatHierarchy_get_ltlspec(hierarchy),
                       FlatHierarchy_get_invarspec(hierarchy)
                       // FlatHierarchy_get_pslspec(hierarchy),
  };
  int i;

  nusmv_assert(dag_info != (hash_ptr) NULL);

  /* extracts info from the FSM */
  Compile_make_dag_info(compile_get_rid_of_define_chain(st,
     FlatHierarchy_get_init(hierarchy), cdh), dag_info);
  Compile_make_dag_info(compile_get_rid_of_define_chain(st,
     FlatHierarchy_get_invar(hierarchy), cdh), dag_info);
  Compile_make_dag_info(compile_get_rid_of_define_chain(st,
     FlatHierarchy_get_trans(hierarchy), cdh), dag_info);
  Compile_make_dag_info(compile_get_rid_of_define_chain(st,
     FlatHierarchy_get_justice(hierarchy), cdh), dag_info);
  Compile_make_dag_info(compile_get_rid_of_define_chain(st,
     FlatHierarchy_get_compassion(hierarchy), cdh), dag_info);

  { /* learn from ASSIGNs */
    Set_t vars = FlatHierarchy_get_vars(hierarchy);
    Set_Iterator_t iter;

    SET_FOREACH(vars, iter) {
      node_ptr name = Set_GetMember(vars, iter);
      node_ptr init_name = find_node(SMALLINIT, name, Nil);
      node_ptr next_name = find_node(NEXT, name, Nil);
      Compile_make_dag_info(compile_get_rid_of_define_chain(st,
         FlatHierarchy_lookup_assign(hierarchy, name), cdh), dag_info);
      Compile_make_dag_info(compile_get_rid_of_define_chain(st,
         FlatHierarchy_lookup_assign(hierarchy, init_name), cdh), dag_info);
      Compile_make_dag_info(compile_get_rid_of_define_chain(st,
         FlatHierarchy_lookup_assign(hierarchy, next_name), cdh), dag_info);
    }
  }

  {
    /* Extracts DAG info from the defines */
    SymbTableIter iter;

    SYMB_TABLE_FOREACH(st, iter, STT_DEFINE) {
      node_ptr define = SymbTable_iter_get_symbol(st, &iter);
#if 0
      node_ptr body = SymbTable_get_define_flatten_body(st, define);
#else
      node_ptr body = SymbTable_get_define_body(st, define);
      node_ptr ctx = SymbTable_get_define_context(st, define);

      if (force_flattening || (Nil != ctx)) {
        body = Compile_FlattenSexp(st, body, ctx);
      }
#endif
      {
        node_ptr b = compile_get_rid_of_define_chain(st, body, cdh);
        if (b != body) body = b;
      }
      Compile_make_dag_info(body, dag_info);
    }
  }

  /* extracts info from the specifications */
  for (i=0; i < sizeof(specs)/sizeof(specs[0]); ++i) {
    node_ptr spec;
    if (det_layer != SYMB_LAYER(NULL)) {
      spec = Compile_expr2bexpr(enc, det_layer, specs[i]);
    }
    else {
      node_ptr ctx;

      if ((Nil != specs[i]) && (Nil != car(specs[i])) &&
          (CONTEXT == node_get_type(car(specs[i])))) {
        ctx = caar(specs[i]);
        if (force_flattening || (Nil != ctx)) {
          spec = Compile_FlattenSexp(st, specs[i], Nil);
        }
        else {
          spec = specs[i];
        }
      }
      else {
        spec = Nil;
      }
    }
    {
      node_ptr s = compile_get_rid_of_define_chain(st, spec, cdh);
      if (s != spec) spec = s;
    }
    Compile_make_dag_info(spec, dag_info);
  }

  return dag_info;
}


/**Function********************************************************************

   Synopsis           []

   Description        [Returns a node COLON(NUMBER count, NUMBER depth)]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr compile_make_dag_info_aux(node_ptr expr, hash_ptr hash)
{
  node_ptr info;

  if (expr == Nil) return compile_pack_dag_info(1, 0, true);
  info = find_assoc(hash, expr);
  if (info != Nil) {
    unsigned int count;
    unsigned int depth;
    boolean admissible;

    compile_unpack_dag_info(info, &count, &depth, &admissible);
    compile_set_dag_info(info, count+1, depth, admissible);
    return info;
  }

  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case TRUEEXP: case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD: case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC: case NUMBER_REAL: case NUMBER_EXP:
  case ATOM: case BIT: case ARRAY: case SELF: case DOT:
    return compile_pack_dag_info(1, 0, true);

    /* cases not to be stored */
  case COLON:
  case NEXT:
    {
      unsigned int count = 0;
      unsigned int depth = 0;
      boolean ladmissible, radmissible;

      node_ptr left = compile_make_dag_info_aux(car(expr), hash);
      node_ptr right = compile_make_dag_info_aux(cdr(expr), hash);
      nusmv_assert(left != Nil || right != Nil); /* cannot be a leaf */
      if (left != Nil) {
        compile_unpack_dag_info(left, &count, &depth, &ladmissible);
      }
      if (right != Nil) {
        unsigned int rdepth;
        compile_unpack_dag_info(right, &count, &rdepth, &radmissible);
        depth = MAX(rdepth, depth);
      }

      return compile_pack_dag_info(1, depth+1, radmissible && ladmissible);
    }

    /* Not admissible cases */
  case OP_NEXT:
  case OP_GLOBAL:
  case OP_FUTURE:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_HISTORICAL:
  case OP_ONCE:
  case EU:
  case AU:
  case EBU:
  case ABU:
  case MINU:
  case MAXU:
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case SINCE:
  case UNTIL:
  case TRIGGERED:
  case RELEASES:
  case EBF:
  case EBG:
  case ABF:
  case ABG:
    {
      unsigned int count = 0;
      unsigned int depth = 0;
      boolean ladmissible, radmissible;

      node_ptr left = compile_make_dag_info_aux(car(expr), hash);
      node_ptr right = compile_make_dag_info_aux(cdr(expr), hash);
      nusmv_assert(left != Nil || right != Nil); /* cannot be a leaf */
      if (left != Nil) {
        compile_unpack_dag_info(left, &count, &depth, &ladmissible);
      }
      if (right != Nil) {
        unsigned int rdepth;
        compile_unpack_dag_info(right, &count, &rdepth, &radmissible);
        depth = MAX(rdepth, depth);
      }

      return compile_pack_dag_info(1, depth+1, false);
    }


  default:
    {
      unsigned int depth = 0;
      unsigned int count = 0; /* this is dummy use */
      node_ptr left = compile_make_dag_info_aux(car(expr), hash);
      node_ptr right = compile_make_dag_info_aux(cdr(expr), hash);
      boolean ladmissible, radmissible;

      nusmv_assert(left != Nil || right != Nil); /* cannot be a leaf */
      if (left != Nil) {
        compile_unpack_dag_info(left, &count, &depth, &ladmissible);
      }
      if (right != Nil) {
        unsigned int rdepth;
        compile_unpack_dag_info(right, &count, &rdepth, &radmissible);
        depth = MAX(rdepth, depth);
      }

      info = compile_pack_dag_info(1, depth+1, ladmissible && radmissible);
      insert_assoc(hash, expr, info);
      return info;
    }
  }
}


/**Function********************************************************************

   Synopsis           []

   Description        [Packs given count and depth into a node]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr compile_pack_dag_info(unsigned int count,
                                      unsigned int depth,
                                      boolean admissible)
{
  return new_node(COLON,
                  new_node(COLON,
                           PTR_FROM_INT(node_ptr, count),
                           PTR_FROM_INT(node_ptr, depth)),
                  PTR_FROM_INT(node_ptr, admissible));
}


/**Function********************************************************************

   Synopsis           []

   Description        [Unpacks given node to count and deptch]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_unpack_dag_info(node_ptr info,
                                    unsigned int* count,
                                    unsigned int* depth,
                                    boolean* admissible)
{
  nusmv_assert(node_get_type(info) == COLON);
  *count = PTR_TO_INT(caar(info));
  *depth = PTR_TO_INT(cdar(info));
  *admissible = (boolean) cdr(info);
}

/**Function********************************************************************

   Synopsis           []

   Description        [Sets count and depth]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_set_dag_info(node_ptr info,
                                 unsigned int count,
                                 unsigned int depth,
                                 boolean admissible)
{
  nusmv_assert(node_get_type(info) == COLON);
  /* setcar and setcdr are admitted here as info was created with new_node,
     and there exist no possibility to gamble the node hash */
  setcar(car(info), PTR_FROM_INT(node_ptr, count));
  setcdr(car(info), PTR_FROM_INT(node_ptr, depth));
  setcdr(info, PTR_FROM_INT(node_ptr, admissible));
}




/**Function********************************************************************

   Synopsis           [Internal service of Compile_destroy_dag_info]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static assoc_retval compile_free_node(char *key, char *data, char * arg)
{
  if (Nil != car((node_ptr)data)) free_node(car((node_ptr) data));
  if (data != (char*) NULL) free_node((node_ptr) data);
  return ASSOC_DELETE;
}

/**Function********************************************************************

   Synopsis           [Internal service of Compile_destroy_dag_info]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static assoc_retval compile_free_define(char *key, char *data, char * arg)
{
  if (data != (char*) NULL) {
    free_node(car((node_ptr) data));
    free_node((node_ptr) data);
  }
  return ASSOC_DELETE;
}


/**Function********************************************************************

   Synopsis           [Prints the obfuscation of the given type]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_symbtype_obfuscated_print(SymbType_ptr type,
                                              FILE* out,
                                              const SymbTable_ptr symb_table,
                                              hash_ptr obfuscation_map) {

  if (SymbType_is_enum(type) && (!SymbType_is_boolean(type))) {
    node_ptr l = SymbType_get_enum_type_values(type);
    fprintf(out, "{");
    while (l != Nil) {
      print_node(out, Compile_obfuscate_expression(symb_table,
                                                   car(l),
                                                   obfuscation_map));
      l = cdr(l);
      if (l != Nil) { fprintf(out, ", "); }
    }
    fprintf(out, "}");
  }
  else if (SymbType_is_array(type)) {
    /* print the array and proceed to the subtype */
      fprintf(out, "array %d..%d of ",
              SymbType_get_array_lower_bound(type),
              SymbType_get_array_upper_bound(type));
      compile_symbtype_obfuscated_print(SymbType_get_array_subtype(type), out,
                                        symb_table, obfuscation_map);
  }
  else {
    SymbType_print(type, out);
  }
}


/**Function********************************************************************

   Synopsis           [Get rids of chain of defines]

   Description        [Get rids of chain of defines until it reaches a
   DEFINE whose body is not atomic (i.e. a variable, a constant, or a
   complex expression). It assumes the expression being flattened.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr compile_get_rid_of_define_chain(SymbTable_ptr st,
                                                node_ptr expr,
                                                hash_ptr cdh) {
  node_ptr res;

  if (Nil == expr) return expr;

  res = find_assoc(cdh, expr);

  if (Nil != res) return res;

  switch(node_get_type(expr)) {
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case FAILURE:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    return expr;

  case ATOM:
  case DOT:
  case ARRAY:
  case BIT:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(st, expr, Nil);
      name = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_constant(rs)) { return name; }
      if (ResolveSymbol_is_var(rs)) { return name; } 
      if (ResolveSymbol_is_define(rs)) {
        res = CompileFlatten_resolve_define_chains(st, name, Nil);
        insert_assoc(cdh, expr, res);
        return res;
      }
      insert_assoc(cdh, expr, expr);
      return expr;
    }
  default:
    res = find_node(node_get_type(expr),
                    compile_get_rid_of_define_chain(st, car(expr), cdh),
                    compile_get_rid_of_define_chain(st, cdr(expr), cdh));
    insert_assoc(cdh, expr, res);
    return res;
  }
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_write_obfuscated_dag_defines(FILE* out,
                                                 const SymbTable_ptr st,
                                                 hash_ptr defines,
                                                 hash_ptr obfuscation_map)
{
  char *key, *value;
  st_generator *gen;
  boolean msg_printed = false;

  st_foreach_item(defines, gen, &key, &value) {
    node_ptr define = (node_ptr) value;
    nusmv_assert(define == Nil || node_get_type(define) == COLON);
    if (define != Nil) {
      unsigned int count = PTR_TO_INT(cdr(define));
      if (count > 0) {
        if (!msg_printed) {
          fprintf(out, "-- Symbols introduced by the dumper:\n");
          msg_printed = true;
        }
        if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
          fprintf(out, "-- occurrences: %d\n", count+1);
        }
        fprintf(out, "DEFINE ");
        print_node(out, Compile_obfuscate_expression(st,
                                                     car(define),
                                                     obfuscation_map));
        fprintf(out, " ;\n\n");
      }
    }
  }
}


/**Function********************************************************************

   Synopsis           [Creates a meaningful name for defines needed for dag printing]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr __create_define_name(SymbTable_ptr st,
                              const char * prefix, node_ptr body) {
  node_ptr name;


#if 0
  TypeChecker_ptr tc;
  SymbType_ptr type;
  SymbLayer_ptr layer = SymbTable_get_layer(st, MODEL_LAYER_NAME);
  int max_len = strlen(prefix) + 20;
  int chars;
  char * buf = ALLOC(char, max_len);
  nusmv_assert(NIL(char) != buf);

  tc = SymbTable_get_type_checker(st);
  type = TypeChecker_get_expression_type(tc, body, Nil);
  if (SymbType_is_word(type)) {
    int size;

    size = SymbType_get_word_width(type);
    if (SymbType_is_signed_word(type)) {
      chars = snprintf(buf, max_len, "%sdsw%d_", prefix, size);
      SNPRINTF_CHECK(chars, max_len);

      type = SymbType_create(SYMB_TYPE_SIGNED_WORD, size);
    }
    else {
      chars = snprintf(buf, max_len, "%sduw%d_", prefix, size);
      SNPRINTF_CHECK(chars, max_len);

      type = SymbType_create(SYMB_TYPE_UNSIGNED_WORD, size);
    }
  }
  else if (SymbType_is_wordarray(type)) {
    chars = snprintf(buf, max_len, "%sdwa_", prefix);
    SNPRINTF_CHECK(chars, max_len);

    fprintf(nusmv_stderr, "__create_define_name: not yet handled type: infinite precision\n");
    error_unreachable_code();
  }
  else if (SymbType_is_infinite_precision(type)) {
    chars = snprintf(buf, max_len, "%sdi_", prefix);
    SNPRINTF_CHECK(chars, max_len);

    fprintf(nusmv_stderr, "__create_define_name: not yet handled type: infinite precision\n");
    error_unreachable_code();
  }
  else if (SymbType_is_boolean(type)) {
    chars = snprintf(buf, max_len, "%sdb_", prefix);
    SNPRINTF_CHECK(chars, max_len);

    type = SymbType_create(SYMB_TYPE_BOOLEAN, Nil);
  }
  else if (SymbType_is_enum(type)) {
    chars = snprintf(buf, max_len, "%se_", prefix);
    SNPRINTF_CHECK(chars, max_len);

    fprintf(nusmv_stderr, "__create_define_name: not yet handled type: infinite precision\n");
    error_unreachable_code();
  }
  else {
    error_unreachable_code();
  }

  name = SymbTable_get_fresh_symbol_name(st, buf);
  // SymbLayer_declare_state_var(name, type);

  FREE(buf);
#else
  name = SymbTable_get_fresh_symbol_name(st, "__expr");
#endif

  return name;
}
