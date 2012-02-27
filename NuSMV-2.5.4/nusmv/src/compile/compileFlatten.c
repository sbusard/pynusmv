/**CFile***********************************************************************

   FileName    [compileFlatten.c]

   PackageName [compile]

   Synopsis    [Flattening of the model.]

   Description [Performs the flattening of the model.

   We start from the module <code>main</code> and we recursively
   instantiate all the modules or processes declared in it.<br>
   Consider the following example: <blockquote>

   MODULE main<br>
   ...<br>
   VAR<br>
   a : boolean;<br>
   b : foo;<br>
   ...<br><br>

   MODULE foo<br>
   VAR <br>
   z : boolean;<br>

   ASSIGN<br>
   z := 1;<br>

   </blockquote>

   The flattening instantiate the module foo in the
   <code>main</code> module. You can refer to the variables
   "<code>z</code>" declared in the module <code>foo</code> after
   the flattening by using the dot notation <code>b.z</code>.]

   SeeAlso     []

   Author      [Marco Roveri]

   Copyright   [
   This file is part of the ``compile'' package of NuSMV version 2.
   Copyright (C) 1998-2005 by CMU and FBK-irst.

   NuSMV version 2 is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   NuSMV version 2 is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
   MA 02111-1307 USA.

   For more information on NuSMV see <http://nusmv.fbk.eu> or
   email to <nusmv-users@fbk.eu>.
   Please report bugs to <nusmv-users@fbk.eu>.

   To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#include "compileInt.h"
#include "FlatHierarchy.h"

#include "symb_table/SymbTable.h"
#include "symb_table/SymbLayer.h"
#include "symb_table/SymbType.h"
#include "symb_table/symb_table.h"
#include "symb_table/ResolveSymbol.h"

#include "parser/symbols.h"
#include "parser/psl/pslNode.h"

#include "utils/ustring.h"
#include "utils/assoc.h"
#include "utils/error.h"
#include "utils/range.h"

#include "hrc/hrc.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum {
  Get_Definition_Mode,
  Expand_Definition_Mode
} Definition_Mode_Type;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

   Synopsis     [Body of define in evaluation]

   Description [Indicates that the body of a define is under the
   flattening, it is usde to discover possible recursive definitions.]

   SideEffects []

   SeeAlso      [Flatten_GetDefinition]

******************************************************************************/
#define BUILDING_FLAT_BODY (node_ptr)-11



/**Macro***********************************************************************

   Synopsis     [Cleans and frees the hash]

   Description [A utility used by internal clean up code for hash tables]

   SideEffects []

   SeeAlso      []

******************************************************************************/
#define FREE_HASH(hash)                         \
  {                                             \
    if (hash != (hash_ptr) NULL) {              \
      free_assoc(hash);                         \
      hash = (hash_ptr) NULL;                   \
    }                                           \
  }

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

   Synopsis    [This variable is locally checked by the (de)initialization code
   of the flattener]

   Description []

******************************************************************************/
static boolean flattener_initialized = false;


/**Variable********************************************************************

   Synopsis    [The internal name of the process-selector input variable.]

   Description [The default name is <tt>PROCESS_SELECTOR_VAR_NAME</tt>.
   If there are processes this variable is given the name in the
   function Compile_FlattenHierarchy. Otherwise (if there are no processes)
   the variable is left Nil.]

******************************************************************************/
node_ptr proc_selector_internal_vname = Nil;

/**Variable********************************************************************

   Synopsis    [The mode to perform variable instantiation.]

   Description [Depending the value of this variable we perform
   instantiation of state variables or input variables.]

******************************************************************************/
static Instantiation_Variables_Mode_Type variable_instantiate_mode =
  State_Variables_Instantiation_Mode;

static void set_variable_instantiation_to_input () {
  variable_instantiate_mode = Input_Variables_Instantiation_Mode;
}

static void set_variable_instantiation_to_state (void) {
  variable_instantiate_mode = State_Variables_Instantiation_Mode;
}

static void set_variable_instantiation_to_frozen (void) {
  variable_instantiate_mode = Frozen_Variables_Instantiation_Mode;
}

static Instantiation_Variables_Mode_Type variable_instantiation_mode_get (void)
{
  return variable_instantiate_mode;
}

/**Variable********************************************************************

   Synopsis    [The expansion mode for definitions is sexp flattening.]

   Description [Depending on the value of this variable, a definition
   is expanded or not during the flattening of a sexp.]

******************************************************************************/
static Definition_Mode_Type definition_mode = Get_Definition_Mode;

void set_definition_mode_to_get () {
  definition_mode = Get_Definition_Mode;
}
void set_definition_mode_to_expand () {
  definition_mode = Expand_Definition_Mode;
}
int definition_mode_is_expand (void) {
  return(definition_mode == Expand_Definition_Mode);
}


/**Variable********************************************************************

   Synopsis    [The hash containing the definition of each module read in.]

   Description [This hash uses the name of the module as index, and for
   each module it stores the following data structure:<br>
   <center><code>&lt;LAMBDA , arguments, module_body&gt;</code></center><br>
   I.e. it is a node, whose type is <code>LAMBDA</code> and whose "car" are
   the module arguments and the "cdr" is the module body (i.e. normal
   assignments, init assignments and next assignments.
   ]

******************************************************************************/
static hash_ptr module_hash;
void insert_module_hash(node_ptr x, node_ptr y)
{ insert_assoc(module_hash, x, y); }

node_ptr lookup_module_hash(node_ptr x)
{
  return(find_assoc(module_hash, x));
}

static void init_module_hash(void)
{
  /* Auxiliary variable used to traverse the parse tree. */
  node_ptr m;
  /* The parse tree representing the input files. */
  extern node_ptr parsed_tree;

  module_hash = new_assoc();
  m = parsed_tree;
  while (m != Nil) {
    node_ptr cur_module = car(m);
    if (Nil != cur_module && node_get_type(cur_module) == MODULE) {
      CompileFlatten_hash_module(cur_module);
    }
    m = cdr(m);
  }
}

static void clear_module_hash(void) { FREE_HASH(module_hash); }


/**Variable********************************************************************

   Synopsis    [The hash of flatten_def]

   Description [This hash associates to an atom corresponding to a
   defined symbol the corresponding flattened body.]

******************************************************************************/
static hash_ptr flatten_def_hash = (hash_ptr)NULL;
static void init_flatten_def_hash()
{
  flatten_def_hash = new_assoc();
  nusmv_assert(flatten_def_hash != (hash_ptr)NULL);
}

static void insert_flatten_def_hash(node_ptr key, node_ptr value)
{
  nusmv_assert(flatten_def_hash != (hash_ptr)NULL);
  insert_assoc(flatten_def_hash, key, (node_ptr)value);
}

static node_ptr lookup_flatten_def_hash(node_ptr key)
{
  nusmv_assert(flatten_def_hash != (hash_ptr)NULL);
  return((node_ptr)find_assoc(flatten_def_hash, key));
}

static assoc_retval flatten_def_hash_free(char *key, char *data, char * arg)
{
  node_ptr element = (node_ptr)data;
  /* Notice that this hash may contain elements set to
     BUILDING_FLAT_BODY in cases of errors inside the flattening
     procedure */
  if (element != (node_ptr)NULL && element != BUILDING_FLAT_BODY) {
    free_node(element);
  }
  return(ASSOC_DELETE);
}

static void clear_flatten_def_hash()
{
  if (flatten_def_hash != (hash_ptr) NULL) {
    /* This was commented out, as this table contains values shared
       among different keys, and some of those values may be
       created with new_node (which would require to free
       them). However, this would end up in freeing multiple times
       the same node */
    /* clear_assoc_and_free_entries(flatten_def_hash,
                                    flatten_def_hash_free); */
    free_assoc(flatten_def_hash);
    flatten_def_hash = (hash_ptr) NULL;
  }
}


/**Variable********************************************************************

   Synopsis    [Variable containing the current context in the
   instantiation phase.]

   Description [Variable containing the current context in the
   instantiation phase. It is used in the instantiation of the
   arguments of modules or processes.]

******************************************************************************/
static node_ptr param_context = Nil;

/**Variable********************************************************************

   Synopsis    [The stack containing the nesting for modules.]

   Description [This variable contains the nesting of modules. It is
   used in the instantiation phase to check for recursively defined modules.]

******************************************************************************/
static node_ptr module_stack = Nil;



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void
compile_instantiate ARGS((SymbTable_ptr st,
                          SymbLayer_ptr,
                          node_ptr,
                          node_ptr,
                          node_ptr,
                          node_ptr*,
                          FlatHierarchy_ptr,
                          HrcNode_ptr,
                          hash_ptr));

static void
compile_instantiate_by_name ARGS((SymbTable_ptr, SymbLayer_ptr,
                                  node_ptr, node_ptr, node_ptr,
                                  node_ptr *, FlatHierarchy_ptr, HrcNode_ptr,
                                  hash_ptr));

static void
compile_add_vars_to_hierarhcy ARGS((node_ptr name, SymbType_ptr type,
                                    FlatHierarchy_ptr fh));
static void
compile_instantiate_var ARGS((SymbTable_ptr st,
                              SymbLayer_ptr layer,
                              node_ptr,
                              node_ptr,
                              node_ptr,
                              node_ptr*,
                              FlatHierarchy_ptr,
                              HrcNode_ptr,
                              hash_ptr));
static void
compile_instantiate_vars ARGS((SymbTable_ptr st,
                               SymbLayer_ptr layer, node_ptr, node_ptr,
                               node_ptr *, FlatHierarchy_ptr, HrcNode_ptr,
                               hash_ptr));

static node_ptr put_in_context ARGS((node_ptr));

static node_ptr
compileFlattenSexpRecur ARGS((const SymbTable_ptr, node_ptr, node_ptr));

static node_ptr
compileFlattenProcess ARGS((const SymbTable_ptr,
                            node_ptr,
                            FlatHierarchy_ptr));

static void
compileFlattenProcessRecur ARGS((const SymbTable_ptr,
                                 node_ptr,
                                 node_ptr,
                                 node_ptr,
                                 FlatHierarchy_ptr));

static node_ptr
compile_flatten_eval_number ARGS((const SymbTable_ptr symb_table,
                                  node_ptr n, node_ptr context));

static void
create_process_symbolic_variables ARGS((SymbTable_ptr symb_table,
                                        SymbLayer_ptr, node_ptr));

static void
flatten_declare_constants_within_list ARGS((SymbTable_ptr symb_table,
                                            SymbLayer_ptr layer,
                                            node_ptr range));

static void resolve_range ARGS((SymbTable_ptr st,
                                node_ptr range, node_ptr context,
                                int* low, int* high));

static void
instantiate_array_define ARGS((SymbTable_ptr st,
                                SymbLayer_ptr layer,
                                node_ptr name,
                                node_ptr mod_name,
                                node_ptr definition));

static node_ptr
construct_array_multiplexer ARGS((node_ptr array, node_ptr index,
                                   boolean is_array_next,
                                   SymbTable_ptr st));

static node_ptr
push_array_index_down ARGS((node_ptr array, node_ptr index,
                              boolean is_array_next,
                              SymbTable_ptr st));

static void compile_insert_assign_hrc ARGS((HrcNode_ptr hrc_result,
                                            node_ptr cur_decl));

static void make_params_hrc ARGS((node_ptr basename,
                                  node_ptr actual_list,
                                  node_ptr formal_list,
                                  HrcNode_ptr hrc_result));

static HrcNode_ptr get_hrc_root_node ARGS((HrcNode_ptr node));

static int compile_flatten_get_int ARGS((node_ptr value));

static node_ptr
compile_flatten_normalise_value_list ARGS((node_ptr old_value_list));


static node_ptr
compile_flatten_build_word_toint_ith_bit_case ARGS((node_ptr wexpr,
                                                    int bit,
                                                    boolean is_negative));
static node_ptr
compile_flatten_rewrite_word_toint_cast ARGS((node_ptr body,
                                              SymbType_ptr type));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [Traverse the module hierarchy, collect all required
   the informations and flatten the hierarchy.]

   Description        [Traverses the module hierarchy and extracts the
   information needed to compile the automaton. The hierarchy of modules
   is flattened, the variables are added to the symbol table, all the
   necessary parts of the model are collected (i.e. the formulae to be
   verified, the initial expressions, etc). Most of the collected
   expressions are flattened.


   The returned value is a structure containing all the collected
   parts. See FlatHierarchy_create function for more info about, and
   constrains on content of the class FlatHierarchy.

   It is the invoker's responsibility to destroy the returned value.

   Parameter `create_process_variables` enables the creation of
   process variable (i.e. declaration of 'running's ). So, this
   parameter can be set up only for users 'main' modules. For auxiliary
   modules created during execution (for example, during LTL tablaue
   generation) this parameter should be set to false (as is done in ltl.c).

   Parameter calc_vars_constr controls the time association between
   constraints and variables is calculated. If true, the association is
   calculated before existing the function, otherwise it is possibly
   calculated later when needed, i.e. when
   FlatHierarchy_lookup_constrains is called. Postponing this calculation
   can be effective when vars constraints are not used in later phases.
   Any value of calc_vars_constr is safe, but having this parameter set
   to false possibly postpones calculations from the model construction
   phase to the model checking phase, when LTL MC is carried out, or when
   COI is involved.

   Parameter hrc_result contains the hrc node to be constructed from the
   model. If hrc_result is NULL then the structure is not populated.]

   SideEffects        [None]

   SeeAlso            []

******************************************************************************/
FlatHierarchy_ptr
Compile_FlattenHierarchy(
  const SymbTable_ptr symb_table,
  SymbLayer_ptr layer, /* the symbolic layer to flat */
  node_ptr module_name,  /* the <code>ATOM</code> representing the
                            name of the module being instantiated (at
                            the top of the hierarchy. */
  node_ptr inst_name, /* the name of the module instance
                         at the top of the hierarchy. */
  node_ptr actual, /* the actual module arguments */
  boolean create_process_variables, /* enables creation of process variables */
  boolean calc_vars_constr, /* triggers calc of vars constr, or delays it */
  HrcNode_ptr hrc_result) /* hrc node to be populated*/
{
  FlatHierarchy_ptr result = FlatHierarchy_create(symb_table);

  /* Take care of redefinitions of module instances. */
  hash_ptr instances = new_assoc();

  /* creation of hrc structure */
  /* warning The way hrc structure is instatiated will be refactored. The
     actual implementation is not definitive */

  if (HRC_NODE(NULL) != hrc_result) {
    node_ptr mod_def = lookup_module_hash(find_atom(module_name));
    if (NODE_PTR(Nil) == mod_def) {
      error_undefined(module_name); /* The module is undefined */
    }

    HrcNode_set_symbol_table(hrc_result, symb_table);
    HrcNode_set_lineno(hrc_result, node_get_lineno(mod_def));
    HrcNode_set_name(hrc_result, module_name);
    HrcNode_set_instance_name(hrc_result, inst_name);
  }


  /* collect all the constructs of a hierarchy */
  Compile_ConstructHierarchy(symb_table, layer, module_name,
                             inst_name, actual, result,
                             hrc_result, instances);

  /* Process the created hierarchy. */
  Compile_ProcessHierarchy(symb_table, layer, result, inst_name,
                           create_process_variables,
                           calc_vars_constr);

  if (FlatHierarchy_get_compassion(result) != Nil) {
    fprintf(nusmv_stdout,
      "WARNING *** The model contains COMPASSION declarations.        ***\n"
      "WARNING *** Full fairness is not yet fully supported in NuSMV. ***\n"
      "WARNING *** Currently, COMPASSION declarations are only        ***\n"
      "WARNING *** supported for BDD-based LTL Model Checking.        ***\n"
      "WARNING *** Results of CTL Model Checking and of Bounded       ***\n"
      "WARNING *** Model Checking may be wrong.                       ***\n");
  }

  if (HRC_NODE(NULL) != hrc_result) {
    if (HrcNode_get_undef(hrc_result) != (void*)NULL) {
      fprintf(nusmv_stdout,
              "WARNING *** The model contains PROCESSes or ISAs. ***\n"
              "WARNING *** The HRC hierarchy will not be usable. ***\n");
    }
  }

  free_assoc(instances);
  return result;
}


/**Function********************************************************************

   Synopsis           [Traverses the module hierarchy and extracts the
   information needed to compile the automaton.]

   Description        [This function is a subfunction of
                       Compile_FlattenHierarchy.

   This function traverses the module hierarchy and extracts the
   information needed to compile the automaton. The hierarchy of modules
   is flattened, the variables are added to the symbol table, all the
   necessary parts of the model are collected (i.e. the formulae to be
   verified, the initial expressions, etc).

   The returned value is a structure constraining all the collected parts
   which are:
   the list of TRANS, INIT, INVAR, ASSIGN, SPEC, COMPUTE, LTLSPEC,
   PSLSPEC, INVARSPEC, JUSTICE, COMPASSION,
   a full list of variables declared in the hierarchy,
   a hash table associating variables to their assignments and constrains.
   See FlatHierarchy class for more info.
   ]

   SideEffects        []

******************************************************************************/
void
Compile_ConstructHierarchy(
  SymbTable_ptr st, /* the symbol table the layer belongs to */
  SymbLayer_ptr layer, /* the layer that must be filled in by the flattening */
  node_ptr module_name, /* the <code>ATOM</code> representing the name of the
                           module being instantiated */
  node_ptr instance_name, /* the name of the module instance to be
                             instantiated */
  node_ptr actual, /* the actual module arguments */
  FlatHierarchy_ptr result,
  HrcNode_ptr hrc_result,
  hash_ptr instances)
{
  node_ptr tmp_assign = Nil;
  compile_instantiate_by_name(st, layer, module_name, instance_name, actual,
                              &tmp_assign, result, hrc_result, instances);

  /* create a list of pairs (process name,assignments in it), it to the result */
  tmp_assign = cons(cons(instance_name, tmp_assign),
                    FlatHierarchy_get_assign(result));
  FlatHierarchy_set_assign(result, tmp_assign);
}


/**Function********************************************************************

   Synopsis           [This function processes a hierarchy after
   collecting all its subparts.]

   Description        [This processing means:
   1. process_selector variable and running defines are declared (only if
   create_process_variables is on)
   2. All the required lists of expressions are reversed.
   All the constrains (not specifications) are flattened.
   3. An association between vars and constrains are created (for ASSIGN,
   INIT, INVAR, TRANS).
   4. Type checking of the variable and define declarations and of all the
   expressions.
   5. Also a correct use of input variables and lack of circular dependences
   are checked.

   The parameters:
   layer is a layer with module variables.
   hierachy is a hierarchy to be process.
   name is a name of the module instance, i.e. a context of all expressions.
   create_process_variables enables creation of process variables.
   ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Compile_ProcessHierarchy(SymbTable_ptr symb_table,
                              SymbLayer_ptr layer,
                              FlatHierarchy_ptr hierarchy,
                              node_ptr name,
                              boolean create_process_variables,
                              boolean calc_vars_constr)
{
  node_ptr tmp;

  /* --- 1 ---- */
  /* if processes are not allowed then no processes should exist */
  nusmv_assert(create_process_variables ||
               (Nil != FlatHierarchy_get_assign(hierarchy) &&
                Nil == cdr(FlatHierarchy_get_assign(hierarchy))));

  /* Create process_selector variables and running defines (if required at all)
     (this must be done before flattening processes and type checking).
  */
  if (create_process_variables) { /* 'map' is used to get all process names */
    tmp = map(car, FlatHierarchy_get_assign(hierarchy));
    create_process_symbolic_variables(symb_table, layer, tmp);
    /* do not free tmp, it is used to construct the type and saved there */
  }

  /* --- 2 ---- */

  /* Flatten the expressions INIT, TRANS, INVAR, JUSTICE and COMPASSION */
  tmp = Compile_FlattenSexp(symb_table,
                            FlatHierarchy_get_init(hierarchy),
                            name);
  FlatHierarchy_set_init(hierarchy, tmp);

  tmp = Compile_FlattenSexp(symb_table,
                            FlatHierarchy_get_trans(hierarchy),
                            name);
  FlatHierarchy_set_trans(hierarchy, tmp);

  tmp = Compile_FlattenSexp(symb_table,
                            FlatHierarchy_get_invar(hierarchy),
                            name);
  FlatHierarchy_set_invar(hierarchy, tmp);

  tmp = Compile_FlattenSexp(symb_table,
                            reverse(FlatHierarchy_get_justice(hierarchy)),
                            name);
  FlatHierarchy_set_justice(hierarchy, tmp);

  tmp = Compile_FlattenSexp(symb_table,
                            reverse(FlatHierarchy_get_compassion(hierarchy)),
                            name);
  FlatHierarchy_set_compassion(hierarchy, tmp);


  /* The SPEC, LTLSPEC, PSLSPEC, INVAR_SPEC, COMPUTE properties are
     simply reversed but NOT flattened. */

  /* RC: comments below are experiments to handle nested relative contexts */
  tmp = reverse(FlatHierarchy_get_spec(hierarchy));
  FlatHierarchy_set_spec(hierarchy, tmp/*compile_fix_nested_context(tmp)*/);

  tmp = reverse(FlatHierarchy_get_ltlspec(hierarchy));
  FlatHierarchy_set_ltlspec(hierarchy, tmp/*compile_fix_nested_context(tmp)*/);

  tmp = reverse(FlatHierarchy_get_invarspec(hierarchy));
  FlatHierarchy_set_invarspec(hierarchy,
                              tmp/*compile_fix_nested_context(tmp)*/);

  tmp = reverse(FlatHierarchy_get_pslspec(hierarchy));
  FlatHierarchy_set_pslspec(hierarchy, tmp/*compile_fix_nested_context(tmp)*/);

  tmp = reverse(FlatHierarchy_get_compute(hierarchy));
  FlatHierarchy_set_compute(hierarchy, tmp/*compile_fix_nested_context(tmp)*/);

  /* --- 3 ---- */
  /* assignments require special management:
     1. they are flattened
     2. running symbols are added to assignments (if required)
     3. a hash of (var-name -> its assignment, invar, init) is created.
  */
  tmp = compileFlattenProcess(symb_table,
                              FlatHierarchy_get_assign(hierarchy),
                              hierarchy);

  FlatHierarchy_set_assign(hierarchy, tmp);

  /* --- 4 ---- */
  /* creation of association between vars and constraints */
  if (calc_vars_constr) {
    /* triggers the calculation of vars constrains */
    FlatHierarchy_calculate_vars_constrains(hierarchy);
  }

  /* --- 5 (optional) ---- */
  if (opt_syntactic_checks_disabled(OptsHandler_get_instance())) {
    fprintf(nusmv_stdout,
            "WARNING *** Input model well-formance check skipped ***\n");
  }
  else {
    /* checks next operator in all the hierarchy */
    Compile_check_next(symb_table, FlatHierarchy_get_init(hierarchy), Nil,
                       false);
    Compile_check_next(symb_table, FlatHierarchy_get_invar(hierarchy), Nil,
                       false);
    Compile_check_next(symb_table, FlatHierarchy_get_trans(hierarchy), Nil,
                       true);
    Compile_check_next(symb_table, FlatHierarchy_get_justice(hierarchy), Nil,
                       false);
    Compile_check_next(symb_table,
                       FlatHierarchy_get_compassion(hierarchy),
                       Nil,
                       false);
    Compile_check_next(symb_table, FlatHierarchy_get_compute(hierarchy), Nil,
                       false);
    Compile_check_next(symb_table, FlatHierarchy_get_spec(hierarchy), Nil,
                       false);
    Compile_check_next(symb_table, FlatHierarchy_get_invarspec(hierarchy), Nil,
                       true);
    Compile_check_next(symb_table, FlatHierarchy_get_ltlspec(hierarchy), Nil,
                       false);
    Compile_check_next(symb_table, FlatHierarchy_get_pslspec(hierarchy), Nil,
                       false);

    /* Check [m]define bodies for nested nexts */
    {
      SymbTableIter iter;

      SYMB_TABLE_FOREACH(symb_table, iter, STT_DEFINE | STT_ARRAY_DEFINE) {
        node_ptr define = SymbTable_iter_get_symbol(symb_table, &iter);
        Compile_check_next(symb_table, define, Nil, true);
        Compile_check_input_next(symb_table, define, Nil);
      }
    }

    /* --- 6 ---- */
    /* type check the obtained module */
    {
      boolean isOk = true;
      isOk =
        isOk &&
        TypeChecker_check_layer(SymbTable_get_type_checker(symb_table), layer);

      /* get rid of module names */
      tmp = map(cdr, FlatHierarchy_get_assign(hierarchy));

      isOk = isOk && TypeChecker_check_constrains(
                                 SymbTable_get_type_checker(symb_table),
                                 FlatHierarchy_get_init(hierarchy),
                                 FlatHierarchy_get_trans(hierarchy),
                                 FlatHierarchy_get_invar(hierarchy),
                                 tmp,
                                 FlatHierarchy_get_justice(hierarchy),
                                 FlatHierarchy_get_compassion(hierarchy));
      free_list(tmp);

      if (!isOk) error_type_system_violation(); /* error */
    }

    /* --- 7 ---- */
    /* if process variable should NOT be created then this is not a user module
       but module generated by NuSMV.
       So the input variables check and check of cycle dependencies of
       assignments may be skipped.
    */
    if (create_process_variables) {
      compileCheckForInputVars(symb_table,
                               FlatHierarchy_get_trans(hierarchy),
                               FlatHierarchy_get_init(hierarchy),
                               FlatHierarchy_get_invar(hierarchy),
                               FlatHierarchy_get_assign(hierarchy),
                               hierarchy);

      Compile_CheckAssigns(symb_table, FlatHierarchy_get_assign(hierarchy));
    }
  }
}


/**Function********************************************************************

   Synopsis           [Builds the flattened version of an expression.]

   Description        [Builds the flattened version of an
   expression. It does not expand defined symbols with the
   corresponding body.]

   SideEffects        []

   SeeAlso            [Flatten_GetDefinition, Compile_FlattenSexpExpandDefine]

******************************************************************************/
node_ptr Compile_FlattenSexp(const SymbTable_ptr symb_table, node_ptr sexp,
                             node_ptr context)
{
  node_ptr result;
  Definition_Mode_Type old_definition_mode = definition_mode;

  set_definition_mode_to_get();
  CATCH {
    result = compileFlattenSexpRecur(symb_table, sexp, context);
  }
  FAIL {
    definition_mode = old_definition_mode;
    rpterr(NULL); /* rethrow */
  }
  definition_mode = old_definition_mode;

  return result;
}

/**Function********************************************************************

   Synopsis           [Flattens an expression and expands defined symbols.]

   Description        [Flattens an expression and expands defined symbols.]

   SideEffects        []

   SeeAlso            [Flatten_GetDefinition, Compile_FlattenSexp]

******************************************************************************/
node_ptr
Compile_FlattenSexpExpandDefine(const SymbTable_ptr symb_table,
                                node_ptr sexp, node_ptr context)
{
  node_ptr result;
  Definition_Mode_Type old_definition_mode = definition_mode;

  set_definition_mode_to_expand();
  CATCH {
    result = compileFlattenSexpRecur(symb_table, sexp, context);
  }
  FAIL {
    definition_mode = old_definition_mode;
    rpterr(NULL); /* rethrow */
  }
  definition_mode = old_definition_mode;

  return result;
}


/**Function********************************************************************

   Synopsis           [Gets the flattened version of an atom.]

   Description        [Gets the flattened version of an atom. If the
   atom is a define then it is expanded. If the definition mode
   is set to "expand", then the expanded flattened version is returned,
   otherwise, the atom is returned.]

   SideEffects        [The <tt>flatten_def_hash</tt> is modified in
   order to memoize previously computed definition expansion.]

******************************************************************************/
node_ptr Flatten_GetDefinition(const SymbTable_ptr symb_table, node_ptr atom)
{
  node_ptr result = Nil;

  if (SymbTable_is_symbol_var(symb_table, atom)) result = atom;
  else if (SymbTable_is_symbol_constant(symb_table, atom)) result = atom;
  else if (SymbTable_is_symbol_define(symb_table, atom)) {
    node_ptr exp = lookup_flatten_def_hash(atom);

    /* Check for circular recursive definitions */
    if (exp == BUILDING_FLAT_BODY) { error_circular(atom); }
    if (exp == (node_ptr) NULL) {
      /* The body of a definition is flattened and the flattening is
         saved.  The flattened body is not returned. */
      insert_flatten_def_hash(atom, BUILDING_FLAT_BODY);
      io_atom_push(atom);
      {
        Definition_Mode_Type old_definition_mode = definition_mode;
        /*
           We need to store the previous definition expansion mode,
           and force it to be expand here since we are attempting to
           expand the body of defined symbols, and we need to to do it
           recursively. If this is not done, then the expansion of
           further defined symbols occurring in the body is not
           performed.
        */
        set_definition_mode_to_expand();

        exp = compileFlattenSexpRecur(symb_table,
                                      SymbTable_get_define_body(symb_table,
                                                                atom),
                                      SymbTable_get_define_context(symb_table,
                                                                   atom));

        definition_mode = old_definition_mode;
      }
      io_atom_pop();
      insert_flatten_def_hash(atom, exp);
    }

    if (definition_mode_is_expand()) result = exp;
    else result = atom;
  }
  else if (SymbTable_is_symbol_array_define(symb_table, atom)) {
    /* ARRAY DEFINE are never expanded. Instead when connected with
       index-subscript (at higher level) it will become a define and
       then expanded */
    result = atom;
  }
  else if (SymbTable_is_symbol_variable_array(symb_table, atom)) {
    /* If this is a symbol-type, return it as it is because it is a
       array or an array */
    result = atom;
  }
  else {
    /* Throw an error */
    error_undefined(atom);
  }
  return result;
}

/**Function********************************************************************

   Synopsis           [Concatenates contexts ctx1 and ctx2]

   Description        [Since contexts are organized bottom-up
   ("a.b.c" becomes

   DOT
   /  \
   DOT   c
   / \
   a   b
   )

   ctx2 is appended to ctx1 by concatenating ctx1 to ctx2. For example
   if ctx1="c.d.e" and ctx2="a.b.c", node 'a' is searched in ctx2, and
   then substituted by

   / ...
   DOT
   /   \
   ->>  DOT   b
   /  \
   (ctx1)  a

   Important: nodes in ctx2 are traversed and possibly recreated with find_node
   ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr CompileFlatten_concat_contexts(node_ptr ctx1, node_ptr ctx2)
{
  int op;
  if (ctx2 == Nil) return node_normalize(ctx1);

  op = node_get_type(ctx2);
  if (op == DOT && car(ctx2) == Nil) {
    return node_normalize(find_node(DOT, ctx1, cdr(ctx2)));
  }

  if (op == ATOM ||
      op == NUMBER) {
    return node_normalize(find_node(DOT, ctx1, ctx2));
  }

  if (op == BIT) {
    return find_node(BIT,
                     CompileFlatten_concat_contexts(ctx1, car(ctx2)),
                     cdr(ctx2));
  }

  return find_node(op,
                   CompileFlatten_concat_contexts(ctx1, car(ctx2)),
                   node_normalize(cdr(ctx2)));
}

/**Function********************************************************************

   Synopsis           [Resets the hashed information about the given symbol]

   Description [This method is used when removing symbols (for example,
   when removing a layer) as some information about that symbol may be
   chached internally to this module. For example this is the case of
   defines, whose flatten body are privately cached within this module.

   If the symbol is not cached or have no associated information, no
   action is taken.  ]

   SideEffects        []

******************************************************************************/
void Flatten_remove_symbol_info(node_ptr name)
{
  /* module hash */
  if (lookup_module_hash(name) != (node_ptr) NULL) {
    insert_module_hash(name, (node_ptr) NULL);
  }

  /* flatten def */
  if (lookup_flatten_def_hash(name) != (node_ptr) NULL) {
    insert_flatten_def_hash(name, (node_ptr) NULL);
  }
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis           [Error message for unsupported feature]

   Description        []

   SideEffects        []

******************************************************************************/
void error_bit_selection_assignment_not_supported(node_ptr name)
{

  extern int yylineno;
  extern FILE* nusmv_stderr;

  yylineno = node_get_lineno(name);

  start_parsing_err();
  fprintf(nusmv_stderr, "Bit selection '");
  print_node(nusmv_stderr, name);
  fprintf(nusmv_stderr, "':\n");
  fprintf(nusmv_stderr,
          "Error: Current version does not support assignment "\
          "of bit selections.\n");
  finish_parsing_err();
}


/**Function********************************************************************

   Synopsis           [Flatten a hierarchy of SMV processes.]

   Description         [This functions takes in input the list of process names
   and their assignments resulting from the instantiation step and
   fills in the hash table (parameter assign_hash) with the
   associations the following form:
   <ul>
   <li><tt>init(var) -> (init_assign)</tt><br>
   where <tt>init_assign</tt> is the right side of the initialization
   assignement of the variable <tt>var</tt>.
   <li><tt>next(var) -> (case P1.running : next_assign_1;
   case P2.running : next_assign_2;
   ...
   var)</tt><br>
   where  <tt>next_assign_i</tt> is the right side of the next
   assignement for the variable <tt>var</tt> in process <tt>i</tt>.
   When other processes not affecting the variable are running,
   the variable stutter.
   If there are no processes the data structure will degenerate
   into <tt>next(var) -> next_assign</tt>.
   <li><tt>var -> (normal_assign)</tt><br>
   where  <tt>normal_assign</tt> is the right side of the
   normal (invariant) assignement for the variable
   <tt>var</tt>.  </ul>

   The parameter proc_assignment_list is a list of pairs
   (process_name, a list of assignment in the process).
   ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr compileFlattenProcess(const SymbTable_ptr symb_table,
                                      node_ptr proc_assign_list,
                                      FlatHierarchy_ptr flattenHierarchy)
{
  node_ptr l;
  node_ptr result = Nil;
  node_ptr running = sym_intern(RUNNING_SYMBOL);
  /* to make the order of processes declarations the same as in an input file,
     reverse the list except the first element (which is "main" module and
     must be at the beginning)
  */
  proc_assign_list = cons(car(proc_assign_list),
                          reverse_ns(cdr(proc_assign_list)));

  for (l = proc_assign_list; l != Nil; l = cdr(l)) { /* Loops over processes */
    ResolveSymbol_ptr rs;
    node_ptr running_name;

    node_ptr process_assignments = Compile_FlattenSexp(symb_table,
                                                       cdr(car(l)), Nil);
    node_ptr process_name = car(car(l));

    rs = SymbTable_resolve_symbol(symb_table, running, process_name);
    running_name = ResolveSymbol_get_resolved_name(rs);

    result = cons(cons(process_name, process_assignments), result);

    compileFlattenProcessRecur(symb_table, process_assignments, Nil,
                               running_name, flattenHierarchy);
  }

  return result;
}


/**Function********************************************************************

   Synopsis           [Returns a range going from a to b]

   Description        [Returns a range going from a to b. An empty range (Nil)
   is returned whether given 'a' is greater than 'b']

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr CompileFlatten_expand_range(int a, int b)
{
  node_ptr res = Nil;

  int i;
  for (i=b ; i>=a ; i--) {
    res = find_node(CONS, find_node(NUMBER, NODE_FROM_INT(i), Nil), res);
  }

  return res;
}


/**Function********************************************************************

   Synopsis           [Takes a list of values and returns the same
   list being normalised]

   Description         [Takes a list of values and returns the same
   list being normalised]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr CompileFlatten_normalise_value_list(node_ptr old_values)
{
  node_ptr values = compile_flatten_normalise_value_list(old_values);

  /* compile_flatten_normalise_value_list returns only a CONS,
     reverted version of the old_values list. We want to keep the
     original ordering */
  values = reverse(values);

  return values;
}


/**Function********************************************************************

   Synopsis           [Resolves the given symbol to be a number]

   Description        [If given symbol is a number, the node is simply
   returned.  If it is a define, the body is
   returned if it is a number. If it is an actuial
   parameter, it is evaluated. Otherwise NULL is
   returned. Notice that returned nodes can be
   NUMBER, NUMBER_SIGNED_WORD or NUMBER_UNSIGNED_WORD.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr CompileFlatten_resolve_number(SymbTable_ptr symb_table,
                                       node_ptr n, node_ptr context)
{
  node_ptr num = compile_flatten_eval_number(symb_table, n, context);
  if (NUMBER == node_get_type(num) ||
      NUMBER_UNSIGNED_WORD == node_get_type(num) ||
      NUMBER_SIGNED_WORD == node_get_type(num)) return num;

  return (node_ptr) NULL; /* not a number! */
}


/**Function********************************************************************

   Synopsis     [Takes an expression, and if it is a define or parameter
   resolves it to the actual expression.]

   Description [Sometimes a define may be equal to another
   define. This function will remove such chain of defines/parameters
   and return the actual expression or a fully resolved variable or
   constant identifier.
   This operation may be considered more like an optimization
   to avoid define chains, eg, during FSM output.

   NEXT is processed not as an expression but as a part of an identifier, i.e.
   its operand will be resolved as well.

   Note that array defines are not resolved to its definition.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr CompileFlatten_resolve_define_chains(const SymbTable_ptr symb_table,
                                              node_ptr expr, node_ptr context)
{
  boolean is_it_next = false;

  /* loop while the exp is still identifier or a context */
  while (CONTEXT == node_get_type(expr) ||
         DOT == node_get_type(expr) ||
         ARRAY == node_get_type(expr) ||
         ATOM == node_get_type(expr) ||
         NEXT == node_get_type(expr)) {

    ResolveSymbol_ptr rs;
    node_ptr resolved;

    if (CONTEXT == node_get_type(expr)) {
      expr = Compile_FlattenSexp(symb_table, expr, context);
      context = Nil;
      continue;
    }

    if (NEXT == node_get_type(expr)) {
      expr = car(expr);
      /* nested next ? */
      nusmv_assert(!is_it_next);
      is_it_next = true;
      continue;
    }

    /* this is an identifier => process it */
    rs = SymbTable_resolve_symbol(symb_table, expr, context);
    resolved = ResolveSymbol_get_resolved_name(rs);

    /* expr is not identifier but expression */
    if (ResolveSymbol_is_undefined(rs)) {
      return Compile_FlattenSexp(symb_table, expr, context);
    }
    else {
      expr = resolved;
      context = Nil;
    }

    /* if the expression is not identifier => return immediately  */
    if (!(DOT == node_get_type(expr) ||
          ARRAY == node_get_type(expr) ||
          ATOM == node_get_type(expr))) {
      /* ...get_resolved_name gets rid of context */
      nusmv_assert(CONTEXT != node_get_type(expr));
      break;
    }

    /* ResolveSymbol_resolve wraps all identifiers into DOT, which
       is useless for symbol constants which are pure ATOM.
       Exception: artificially created symbolic constants may have DOT.
       This happens, for examples, with constants used to encode
       processes.
    */
    /* artificial constants (with DOT added by ResolveSymbol_resolve) */
    if (ResolveSymbol_is_constant(rs)) {
      break;
    }

    /* a variable */
    else if (ResolveSymbol_is_var(rs)) {
      break;
    }

    /* an array variable */
    else if (ResolveSymbol_is_array(rs)) {
      break;
    }


    /* it is a normal constant.
       Remove DOT added uselessly by ResolveSymbol_resolve */
    else if (DOT == node_get_type(expr) &&
             ATOM == node_get_type(cdr(expr)) &&
             ResolveSymbol_is_constant(rs)) {
      expr = cdr(expr);
      is_it_next = false;
      break;
    }

    /* is it parameter => resolve and continue */
    else if (ResolveSymbol_is_parameter(rs)) {
      context = SymbTable_get_actual_parameter_context(symb_table, expr);
      expr = SymbTable_get_actual_parameter(symb_table, expr);

      /* Flatten expression if needed */
      if (context != Nil) {
        expr = Compile_FlattenSexp(symb_table, expr, context);
        context = Nil;
      }

      continue;
    }

    /* is it define => resolve and continue */
    else if (ResolveSymbol_is_define(rs)) {
      context = SymbTable_get_define_context(symb_table, expr);
      expr = SymbTable_get_define_body(symb_table, expr);
      nusmv_assert(expr != Nil);

      /* Flatten expression if needed */
      if (context != Nil) {
        expr = Compile_FlattenSexp(symb_table, expr, context);
        context = Nil;
      }

      continue;
    }

    /* is it array define => it is not resolved because
       array expression are not yet allowed to be in arbitrary places */
    else if (ResolveSymbol_is_array_def(rs)) {
      break;
    }

    /* this is array expression. Still it potentially may be
       resolved to define, e.g. define v := [1,2,3], define d:=v, then
       d[1] is array expression but can be resolved to 2.
    */
    else if (ARRAY == node_get_type(expr) &&
             !SymbTable_is_symbol_declared(symb_table, expr)) {
      node_ptr tmp = Compile_FlattenSexp(symb_table, expr, context);
      nusmv_assert(tmp != expr); /* loop in recursion */
      expr = tmp;
      context = Nil;

      continue;
    }

    else {
      /* no other id are possible */
      rpterr("\nUnknown (%s) identifier : %s\n",
             SymbTable_is_symbol_declared(symb_table, expr) ?
             "declared" : "undeclared",
             sprint_node(expr));
      error_unreachable_code();
    }
  }

  /* Re-add the NEXT if needed */
  if (is_it_next) {
    expr = Expr_next(expr, symb_table);
  }

  /* If the expression is not flattened */
  if (Nil != context) {
    expr = Compile_FlattenSexp(symb_table, expr, context);
  }

  return expr;
}


/**Function********************************************************************

   Synopsis           [Inits the flattener module]

   Description        [Inits all the internal structures, in order to correctly
   bootstrap the flattener ]

   SideEffects [This module will be initialized, all previously
   iniitalized data will be lost]

   SeeAlso            []

******************************************************************************/
void CompileFlatten_init_flattener()
{
  nusmv_assert(!flattener_initialized); /* not already initialized */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Initializing the flattener...\n");
  }

  init_module_hash();
  init_flatten_def_hash();
  flattener_initialized = true;
}


/**Function********************************************************************

   Synopsis           [Quits the flattener module]

   Description        [Resets all internal structures, in order to correctly
   shut down the flattener. Calls clear_* local functions, and resets all
   private variables. ]

   SideEffects [This module will be deinitialized, all previously
   iniitalized data will be lost]

   SeeAlso            []

******************************************************************************/
void CompileFlatten_quit_flattener()
{
  /* deinits the flattener only if previously initialized */
  if (!flattener_initialized) return;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "Clearing the flattener...\n");
  }

  clear_module_hash();
  clear_flatten_def_hash();

  /* ---------------------------------------------------------------------- */
  /*                        Reseting of variables                           */
  /* ---------------------------------------------------------------------- */

  /* lists: */
  free_list(module_stack);
  module_stack = Nil;

  /* simple nodes: */
  if (proc_selector_internal_vname != Nil) {
    free_node(proc_selector_internal_vname);
    proc_selector_internal_vname = Nil;
  }

  if (param_context != Nil) {
    free_node(param_context);
    param_context = Nil;
  }

  /* other vars: */
  variable_instantiate_mode = State_Variables_Instantiation_Mode;
  definition_mode = Get_Definition_Mode;

  flattener_initialized = false;
}


/**Function********************************************************************

   Synopsis           [Add the tableau module to the list of known modules]

   Description [Add the tableau module (coming from parser) to the
   list of known modules. After this function has been invoked, the
   module will be recognized by the flattener]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void CompileFlatten_hash_module(node_ptr parsed_module)
{
  /* We insert the definition of the current module in the module_hash
     in order to make it available for the Compile_FlattenHierarchy
     routines. */
  node_ptr name = find_atom(caar(parsed_module));
  node_ptr params = cdar(parsed_module);
  node_ptr def = cdr(parsed_module);

  if (lookup_module_hash(name)) error_redefining(name);
  insert_module_hash(name, new_lined_node(LAMBDA, params, reverse(def),
                                          node_get_lineno(parsed_module)));
}


/**Function********************************************************************

   Synopsis   [convert a type from node_ptr-form constructed by parser
   into not-memory-shared SymbType_ptr.]

   Description [All normal simple and complex types can be processed.

   Note that PROCESS and MOD_TYPE are not types and cannot be processed here.
   Parameter:
   st -- is symbol table where constants met in type can be evaluated.
   layer -- is layer where constants will be declared (for enum types).
   type -- is the type to be converted.
   name -- is the name of variable a given type is processed for.
       It is used only in error messaged and also additional checks
       are done wrt special var _process_selector_.

   If type is constructed incorrectly then error is raise. I.e. NULL
   is never returned.

   NOTE: An invoker has to free the returned type.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
SymbType_ptr Compile_InstantiateType(SymbTable_ptr st, SymbLayer_ptr layer,
                                     node_ptr name, node_ptr type,
                                     node_ptr context)
{
  yylineno = node_get_lineno(type);
  SymbType_ptr symbolicType = SYMB_TYPE(NULL);

  /* process the type */
  switch (node_get_type(type)) {
  case BOOLEAN:
    symbolicType = SymbType_create(SYMB_TYPE_BOOLEAN, Nil);
    break;

  case TWODOTS: {
    node_ptr expanded_range = Nil;
    int dim1, dim2;

    resolve_range(st, type, context, &dim1, &dim2);

    /* Checks if the range is a "range", i.e. a range is from "a" to "b"
       with the constraint that "b >= a" */
    expanded_range = CompileFlatten_expand_range(dim1, dim2);
    if (expanded_range == Nil) { error_empty_range(name, dim1, dim2); }
    flatten_declare_constants_within_list(st, layer, expanded_range);

    symbolicType = SymbType_create(SYMB_TYPE_ENUM, expanded_range);
    break;
  }

  case SCALAR: {
    node_ptr value_list = CompileFlatten_normalise_value_list(car(type));
    node_ptr iter;

    /* check that all symbolic constants are not DOTs since only
       process_selector may have complex symbolic constants.
       Also TRUE and FALSE cannot be part of the constants
    */
    for (iter = value_list; Nil != iter; iter = cdr(iter)) {
      /* NOTE: the name of process_selector is the same as in
         create_process_symbolic_variables
      */
      if (DOT == node_get_type(car(iter)) &&
          name != find_node(DOT, Nil, sym_intern(PROCESS_SELECTOR_VAR_NAME))) {
        yylineno = node_get_lineno(car(iter));
        rpterr("unexpected \".\" in a costant name \n");
      }

      if (TRUEEXP == node_get_type(car(iter)) ||
          FALSEEXP == node_get_type(car(iter))) {
        error_invalid_enum_value(car(iter));
      }
    }

    flatten_declare_constants_within_list(st, layer, value_list);

    symbolicType = SymbType_create(SYMB_TYPE_ENUM, value_list);

    break;
  }

  case INTEGER:
    symbolicType = SymbType_create(SYMB_TYPE_INTEGER,  Nil /*body*/);
    break;

  case REAL:
    symbolicType = SymbType_create(SYMB_TYPE_REAL, Nil /*body*/);
    break;

  case UNSIGNED_WORD:
  case SIGNED_WORD: {
    /* the correctness of the width-expression is checked
       in the type-checking phase.
       Then the width expression is evaluated and checked for
       being constant NUMBER.
    */
    node_ptr num = CompileFlatten_resolve_number(st, car(type), context);
    if ((node_ptr) NULL == num || NUMBER != node_get_type(num)) {
      error_not_constant_width_of_word_type(name);
    }
    symbolicType = SymbType_create(node_get_type(type) == SIGNED_WORD ?
                                   SYMB_TYPE_SIGNED_WORD : SYMB_TYPE_UNSIGNED_WORD,
                                   num);

    break;
  }

  case WORDARRAY: {
    /* the correctness of the width-expressions are checked
       in the type-checking phase.
       Then the width expressions are evaluated and checked for
       being constant NUMBER.
    */
    node_ptr dim1 = CompileFlatten_resolve_number(st, car(type), context);
    node_ptr dim2 = CompileFlatten_resolve_number(st, cdr(type), context);

    if (NUMBER != node_get_type(dim1) || NUMBER != node_get_type(dim2)) {
      error_not_constant_width_of_word_array_type(name);
    }

    symbolicType = SymbType_create(SYMB_TYPE_WORDARRAY,
                                   new_lined_node(CONS, dim1, dim2,
                                                  node_get_lineno(type)));
    break;
  }
    /* Array or matrix type */
  case ARRAY_TYPE: {
    node_ptr tmp;
    node_ptr lower, upper;
    int lower_num, upper_num;
    SymbType_ptr subtype;

    /* Resolve the associated range and the subtype */
    tmp = car(car(type));
    lower = CompileFlatten_resolve_number(st, tmp, context);
    if ((node_ptr) NULL == lower) {
      /* error handling */
      extern int yylineno;

      fprintf(nusmv_stderr, "Unexpected value at token '");
      print_node(nusmv_stderr, tmp);
      fprintf(nusmv_stderr, "'\n");
      yylineno = node_get_lineno(tmp);
      error_expected_number();
    }

    tmp = cdr(car(type));
    upper = CompileFlatten_resolve_number(st, tmp, context);
    if ((node_ptr) NULL == upper) {
      /* error handling */
      extern int yylineno;

      fprintf(nusmv_stderr, "Unexpected value at token '");
      print_node(nusmv_stderr, tmp);
      fprintf(nusmv_stderr, "'\n");
      yylineno = node_get_lineno(tmp);
      error_expected_number();
    }

    /* here lower and upper are resolved to numbers */
    lower_num = compile_flatten_get_int(lower);
    upper_num = compile_flatten_get_int(upper);

    subtype = Compile_InstantiateType(st, layer,
                                      find_node(ARRAY, name, lower),
                                      cdr(type), context);

    symbolicType = SymbType_create_array(subtype, lower_num, upper_num);
    break;
  }

  case MODTYPE:
  case PROCESS:
  default:
    internal_error("Compile_InstantiateType: type = %d",
                   node_get_type(type));
    break;
  } /* switch */

  nusmv_assert(NULL != symbolicType);
  return symbolicType;
}



/**Function********************************************************************

   Synopsis    [Instantiates the given variable.]

   Description [It takes as input a variable name, its type and a
   context, and depending on the type of the variable some operation
   are performed in order to instantiate it in the given context:

   Depending on the kind of variable instantiation mode the variables
   are appended to <tt>input_variables</tt>, <tt>frozen_variables</tt> or
   <tt>state_variables</tt>, respectively.

   Note that if type is ARRAY then the "name" is declared
   with SymbLayer_declare_variable_array and then subvariables are
   created.

   Returns true iff a variable (input,state or frozen) or array was
   created.

   PRECONDITION: type has to be not memory-shared, and its ownership
   is passed to this function.
   ]

   SideEffects        []

   SeeAlso            [compile_instantiate_var]

******************************************************************************/
boolean Compile_DeclareVariable(SymbTable_ptr st, SymbLayer_ptr layer,
                                node_ptr name, SymbType_ptr type,
                                node_ptr context,
                                Instantiation_Variables_Mode_Type mode)
{
  boolean result = false;
  ResolveSymbol_ptr rs;

  rs = SymbTable_resolve_symbol(st, name, context);
  name = ResolveSymbol_get_resolved_name(rs);

  /* Detect name clashes between Nil-context vars and constants */
  if ((DOT == node_get_type(name)) && (Nil == car(name)) &&
      (!SymbLayer_can_declare_constant(layer, cdr(name))))  {
    error_ambiguous(name);
  }

  if (!SymbLayer_can_declare_var(layer, name)) {
    /* a more precise error message */
    if (SymbTable_is_symbol_parameter(st, name)) error_shadowing(name);
    else error_redefining(name);
  }

  /* process special cases.
     If one of special cases is detected then required declarations are done
     in below switch and yype is set to NULL. */
  switch (SymbType_get_tag(type)) {
  case SYMB_TYPE_ENUM: {
    /* special case is enumeration of 1 value => declare a define
       instead of a var */
    node_ptr values = SymbType_get_enum_type_values(type);
    nusmv_assert(values != 0); /* every enum type has some values */
    if (!(opt_keep_single_value_vars(OptsHandler_get_instance())) &&
        (Nil == cdr(values))) {
      SymbLayer_declare_define(layer, name, context, car(values));
      SymbType_destroy(type);
      type = SYMB_TYPE(NULL);

      fprintf(nusmv_stderr, "WARNING: single-value variable '");
      print_node(nusmv_stderr, name);
      fprintf(nusmv_stderr, "' has been stored as a constant\n");
    }
    break;
  }

  case SYMB_TYPE_BOOLEAN:
  case SYMB_TYPE_INTEGER:
  case SYMB_TYPE_REAL:
  case SYMB_TYPE_SIGNED_WORD:
  case SYMB_TYPE_UNSIGNED_WORD:
  case SYMB_TYPE_WORDARRAY:
    break; /* nothing special here */

  case SYMB_TYPE_ARRAY: {
    /* Array is fully special:
       Declare the array as a known symbol in this layer which is not a
       variable nor a constant nor a define.
       The recursively declare array elements.*/
    result = true;
    SymbLayer_declare_variable_array(layer, name, type);

    SymbType_ptr subtype = SymbType_get_array_subtype(type);
    int lower = SymbType_get_array_lower_bound(type);
    int upper = SymbType_get_array_upper_bound(type);
    int i;


    for (i=lower; i<=upper; ++i) {
      node_ptr index = find_node(NUMBER, NODE_FROM_INT(i), Nil);
      if (SymbLayer_can_declare_constant(layer, index)) {
        SymbLayer_declare_constant(layer, index);
      }
      /* Creates the name[i] variable (which could be an array as well)*/
      Compile_DeclareVariable(st, layer, find_node(ARRAY, name, index),
                              SymbType_copy(subtype), context, mode);
    }
    type = NULL; /* all declarations are done */
    break;
  }

  default:
    error_unreachable_code(); /* unsupported type */
  };

  /* if type is not NULL then this is a normal case
     and a variable has to be declared */
  if (SYMB_TYPE(NULL) != type) {
    result = true;
    switch (mode) {
    case State_Variables_Instantiation_Mode:
      SymbLayer_declare_state_var(layer, name, type);
      break;
    case Input_Variables_Instantiation_Mode:
      SymbLayer_declare_input_var(layer, name, type);
      break;
    case Frozen_Variables_Instantiation_Mode:
      SymbLayer_declare_frozen_var(layer, name, type);
      break;
    default:
      error_unreachable_code(); /* impossible mode */
      break;
    }
  }

  return result;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



/**Function********************************************************************

   Synopsis   [Given a fully resolved array name and its type the function
   adds all the variables in the array to the hierarchy]

   Description []

   SideEffects []

   SeeAlso     []

******************************************************************************/
static void compile_add_vars_to_hierarhcy(node_ptr name, SymbType_ptr type,
                                          FlatHierarchy_ptr fh)
{
  nusmv_assert(SymbType_is_array(type));

  SymbType_ptr subtype = SymbType_get_array_subtype(type);
  int lower = SymbType_get_array_lower_bound(type);
  int upper = SymbType_get_array_upper_bound(type);
  int i;

  for (i=lower; i<=upper; ++i) {
    node_ptr index = find_node(NUMBER, NODE_FROM_INT(i), Nil);
    node_ptr new_name = find_node(ARRAY, name, index);
    if (SymbType_is_array(subtype)) { /* still array => recursively go down */
      compile_add_vars_to_hierarhcy(new_name, subtype, fh);
    }
    else { /* a var was reached => add to hierarchy */
      FlatHierarchy_add_var(fh, new_name);
    }
  } /* for */
}


/**Function********************************************************************

   Synopsis           [Instantiates the given variable.]

   Description        [It takes as input a variable and a context, and
   depending on the type of the variable some operation are performed in order
   to instantiate it in the given context:
   <br><br>
   <ul>
   <li><b>BOOLEAN</b><br>
   if the variable is of type boolean, then we add an entry in
   <code>symbol_hash</code> saying that the variable values are <code>{0,1}
   </code>.</li>
   <li><b>RANGE</b><br>
   if the variable is a range of the form <code>M..N</code>, then
   we add an entry in the <code>symbol_hash</code> saying that the
   variable values are <code>{M, M+1, ..., N-1, N}</code>. If
   <code>M</code> is less or equal to <code>N</code>, than an error occurs.
   </li> <li><b>ENUMERATION</b><br>
   if the variable is a scalar variable whose possible values are
   <code>{a1, a2, ..., aN}</code>, then we add an entry in the
   <code>symbol_hash</code> saying that the variable values are
   <code>{a1, ..., aN}</code>. </li>
   <li><b>ARRAY</b><br>
   for each element of the array it is created the corresponding
   symbol. Suppose you have the following definition "<code>VAR
   x : array 1..4 of boolean;</code>". We call this function
   for 4 times, asking at each call <code>i</code> (<code>i</code> from 1
   to 4) to declare the boolean variable <code>x\[i\]</code>.</li>
   <li><b>MODULE</b><br>
   If the variable is an instantiation of a module, than their
   arguments (if any) are contextualized, and passed as actual
   parameter to <code>instantiate_by_name<code> with the name of the
   instantiated module as root name (to extract its definition)
   and as variable name as the name of the module (to perform
   flattening).</li>
   <li><b>PROCESS</b><br>
   If the variable is of type process, than we extract the
   module name and args, we perform the contextualization of the
   process arguments and we perform a call to
   <tt>Compile_ConstructHierarchy</tt> using the variable name as process
   name (to perform flattening), the module name as root name (to
   extract its definition) and the computed actual parameters.</li>
   </ul><br>

   Depending on the kind of variable instantiation mode the variables of
   type boolean, scalar, and array are appended to <tt>input_variables</tt>,
   <tt>frozen_variables</tt> or <tt>state_variables</tt>, respectively.
   ]

   SideEffects        []

   SeeAlso            [compile_instantiate_vars]

******************************************************************************/
static void compile_instantiate_var(SymbTable_ptr st,
                                    SymbLayer_ptr layer, node_ptr name,
                                    node_ptr type, node_ptr context,
                                    node_ptr *assign, FlatHierarchy_ptr result,
                                    HrcNode_ptr hrc_result, hash_ptr instances)
{
  node_ptr hrc_var_name;
  ResolveSymbol_ptr rs;

  /* Resolve the module name in a standard way.. */
  node_ptr name_mod = find_node(MODTYPE, find_atom(context), find_atom(name));

  rs = SymbTable_resolve_symbol(st, name, context);

  yylineno = node_get_lineno(name);

  hrc_var_name = name;   /* Name without context, used to build hrc */

  name = ResolveSymbol_get_resolved_name(rs);

  /* Check if the variable can be declared, and an instance with the
     same name does not exist! */
  if (!SymbLayer_can_declare_var(layer, name) ||
      (Nil != find_assoc(instances, name_mod))) {
    /* more precise error message */
    if (ResolveSymbol_is_parameter(rs)) error_shadowing(name);
    else error_redefining(name);
  }

  /* process the type */
  switch (node_get_type(type)) {
    /* Basic types */
  case BOOLEAN:
  case TWODOTS:
  case SCALAR:
  case INTEGER:
  case REAL:
  case UNSIGNED_WORD:
  case SIGNED_WORD:
  case WORDARRAY: {
    SymbType_ptr symbolicType = Compile_InstantiateType(st, layer,
                                                        name, type, context);

    boolean dv = Compile_DeclareVariable(st,
                                         layer,
                                         name,
                                         symbolicType,
                                         context,
                                         variable_instantiation_mode_get());

    if (true == dv) {
      FlatHierarchy_add_var(result, name);
    }

    if (HRC_NODE(NULL) != hrc_result) {
      if (true == dv) {
        Instantiation_Variables_Mode_Type mode =
          variable_instantiation_mode_get();

        node_ptr hrc_var = cons(hrc_var_name, type);

        switch (mode) {
        case State_Variables_Instantiation_Mode:
          HrcNode_add_state_variable(hrc_result, hrc_var);
          break;
        case Input_Variables_Instantiation_Mode:
          HrcNode_add_input_variable(hrc_result, hrc_var);
          break;
        case Frozen_Variables_Instantiation_Mode:
          HrcNode_add_frozen_variable(hrc_result, hrc_var);
          break;
        default:
          internal_error("compile_instantiate_var: impossible mode");
          break;
        }
      }
      /* Check for single-value enums declared as constants to be
         declared also into the HRC node */
      else if (SymbTable_is_symbol_define(st, name)) {
          node_ptr body = SymbTable_get_define_body(st, name);

          if (NUMBER != node_get_type(body)) {
            HrcNode_add_constants(hrc_result, cons(body, Nil));
            HrcNode_add_define(hrc_result, cons(hrc_var_name, body));
          }
          else {
            HrcNode_add_define(hrc_result, cons(hrc_var_name, body));
          }
      }
    }

    break;
  }

    /* Array or matrix type */
  case ARRAY_TYPE: {
    SymbType_ptr symbolicType = Compile_InstantiateType(st, layer,
                                                         name, type, context);
    /* SymbLayer_declare_variable_array will be invoked in
       Compile_DeclareVariable */
    Compile_DeclareVariable(st, layer, name, symbolicType, context,
                            variable_instantiation_mode_get());

    compile_add_vars_to_hierarhcy(name, symbolicType, result);

    if (HRC_NODE(NULL) != hrc_result) {
      Instantiation_Variables_Mode_Type mode =
        variable_instantiation_mode_get();

      node_ptr hrc_var = cons(hrc_var_name, type);

      switch (mode) {
      case State_Variables_Instantiation_Mode:
        HrcNode_add_state_variable(hrc_result, hrc_var);
        break;
      case Input_Variables_Instantiation_Mode:
        HrcNode_add_input_variable(hrc_result, hrc_var);
        break;
      case Frozen_Variables_Instantiation_Mode:
        HrcNode_add_frozen_variable(hrc_result, hrc_var);
        break;
      default:
        internal_error("compile_instantiate_var: impossible mode");
        break;
      }
      /*
         Note that for array only entire array variable (array r..m of
         type) is kept in the hrc structure. So single variables
         created by the flattener for the array are NOT contained in
         the hrc structure.
      */
    }

    break;
  }

    /* Module Instantiation */
  case MODTYPE: {
    node_ptr actual;

    param_context = context;
    actual = map(put_in_context, cdr(type));

    /* Insert the instance in the instances map */
    insert_assoc(instances, name_mod, NODE_FROM_INT(1));

    if (HRC_NODE(NULL) == hrc_result) {
      compile_instantiate_by_name(st, layer, car(type), name, actual,
                                  assign, result, HRC_NODE(NULL),
                                  instances);
    }
    else {
      HrcNode_ptr hrc_child;
      node_ptr crude_mod_name;
      node_ptr mod_name;
      node_ptr mod_def;

      /* In this way a unique reference to the module name is used,
         so the module name can be used as a key. */
      crude_mod_name = car(type);
      mod_name = find_atom(crude_mod_name);

      mod_def  = lookup_module_hash(mod_name);
      if (NODE_PTR(Nil) == mod_def) {
        /* The module is undefined */
        error_undefined(mod_name);
      }

      hrc_child = HrcNode_create();
      HrcNode_set_symbol_table(hrc_child, st);
      HrcNode_set_name(hrc_child, crude_mod_name);
      HrcNode_set_lineno(hrc_child, node_get_lineno(mod_def));
      HrcNode_set_instance_name(hrc_child, hrc_var_name);
      HrcNode_set_parent(hrc_child, hrc_result);
      HrcNode_add_child_hrc_node(hrc_result, hrc_child);

      /* Adds formal/actual parameters to the module instance.
         This step is not performed in the make_params function
         because non-flattened actual parameters are needed in hrc
         structure.
       */
      make_params_hrc(name,
                      cdr(type) /* non-flattened actual parameters */,
                      car(mod_def),
                      hrc_child);

      compile_instantiate_by_name(st, layer, car(type), name, actual,
                                  assign, result, hrc_child, instances);
    }
    free_list(actual);
    break;
  }

    /* Module process instantiation */
  case PROCESS: {

    if (HRC_NODE(NULL) != hrc_result) {
      HrcNode_ptr root = get_hrc_root_node(hrc_result);
      /* Set a flag so we know that the HRC hierarchy is not usable */
      HrcNode_set_undef(root, (void*)~0);
      hrc_result = HRC_NODE(NULL);
    }
    node_ptr actual;
    node_ptr pmod_name = car(car(type));
    node_ptr pmod_args = cdr(car(type));

    param_context = context;
    actual = map(put_in_context, pmod_args);

    Compile_ConstructHierarchy(st,
                               layer,
                               pmod_name,
                               name,
                               actual,
                               result,
                               hrc_result,
                               instances);
    free_list(actual);
    break;
  }

  default:
    internal_error("compile_instantiate_var: type = %d",
                   node_get_type(type));
    break;
  }
}


/**Function********************************************************************

   Synopsis           [Recursively applies <tt>compile_instantiate_var</tt>.]

   Description        [Recursively applies <tt>compile_instantiate_var</tt> to
   a given list of variables declaration, and performs some check for
   multiple variable definitions.]

   SideEffects        []

   SeeAlso            [compile_instantiate_var]

******************************************************************************/
static void compile_instantiate_vars(SymbTable_ptr st,
                                     SymbLayer_ptr layer, node_ptr var_list,
                                     node_ptr mod_name,
                                     node_ptr *assign,
                                     FlatHierarchy_ptr result,
                                     HrcNode_ptr hrc_result,
                                     hash_ptr instances)
{
  node_ptr rev_vars_list;
  node_ptr iter;

  rev_vars_list = reverse_ns(var_list);
  iter = rev_vars_list;
  while (iter != Nil) {
    node_ptr cur_var = car(iter);
    node_ptr name = car(cur_var);
    node_ptr type = cdr(cur_var);

    compile_instantiate_var(st,
                            layer,
                            name,
                            type,
                            mod_name,
                            assign,
                            result,
                            hrc_result,
                            instances);

    iter = cdr(iter);
  }

  free_list(rev_vars_list);
}


/**Function********************************************************************

   Synopsis           [This function takes a TWODOTS node, and tries to resolve
   the bounds to integer numbers which are returned.]

   Description        [If it is not possible to resolve the bounds to numbers,
   an error is issued.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void resolve_range(SymbTable_ptr st,
                          node_ptr range, node_ptr context,
                          int* low, int* high)
{
  node_ptr ndim;

  nusmv_assert(TWODOTS == node_get_type(range));

  ndim = CompileFlatten_resolve_number(st, car(range), context);

  if ((node_ptr) NULL == ndim || NUMBER != node_get_type(ndim)) {
    yylineno = node_get_lineno(range);
    error_invalid_subrange(range);
  }
  *low = node_get_int(ndim);

  ndim = CompileFlatten_resolve_number(st, cdr(range), context);
  if ((node_ptr) NULL == ndim || NUMBER != node_get_type(ndim)) {
    yylineno = node_get_lineno(range);
    error_invalid_subrange(range);
  }
  *high = node_get_int(ndim);
}


/**Function********************************************************************

   Synopsis           [Put a variable in the current "context"]

   Description        [Put a variable in the current "context", which
   is stored in <code>param_context</code>.]

   SideEffects        [None]

   SeeAlso            [param_context]

******************************************************************************/
static node_ptr put_in_context(node_ptr v)
{
  return(find_node(CONTEXT, param_context, v));
}


/**Function********************************************************************

   Synopsis [Builds the parameters of a module from the list of formal
   parameters of the module itself.]

   Description        [Builds the parameters of a module from the list
   of formal parameters of the module itself and a <tt>basename</tt>.<br>
   There must be a one to one correspondence between the elements of
   <tt>actual_list</tt> and <tt>formal_list</tt> parameters. If the
   number of elements of the lists are different then, an error occurs.]

   SideEffects        [In the symbol table the new parameter is
   associated to the old one.]

******************************************************************************/
static void make_params(SymbLayer_ptr layer,
                        node_ptr basename,
                        node_ptr actual_list,
                        node_ptr formal_list)
{
  /* DO NOT CHANGE yylineno, now it points to declared instance basename */

  while (formal_list) {
    node_ptr actual_parameter, formal_parameter_flat;

    if (!actual_list) {
      if (basename != (node_ptr) NULL) {
        fprintf(nusmv_stderr, "While creating instance ");
        print_node(nusmv_stderr, basename);
        rpterr("too few actual parameters");
      }
      else {
        rpterr("module 'main' cannot have formal parameters");
      }
    }

    /* get the current actual and formal parameters */
    formal_parameter_flat =
      find_node(DOT, basename, find_atom(car(formal_list)));

    formal_parameter_flat = node_normalize(formal_parameter_flat);

    actual_parameter = car(actual_list);

    if (!SymbLayer_can_declare_parameter(layer, formal_parameter_flat)) {
      yylineno = node_get_lineno(car(formal_list));
      error_redefining(formal_parameter_flat);
    }

    SymbLayer_declare_parameter(layer, formal_parameter_flat,
                                car(actual_parameter),
                                cdr(actual_parameter));

    /* advance actual and formal lists */
    formal_list = cdr(formal_list);
    actual_list = cdr(actual_list);
  }
  if (actual_list) rpterr("too many actual parameters");
}

/**Function********************************************************************

Synopsis [Builds the parameters of a module from the list of formal
and actual parameters of the module itself.]

Description        [Builds the parameters of a module from the list
of formal parameters of the module itself.<br>There must be a one to
one correspondence between the elements of <tt>actual_list</tt> and
<tt>formal_list</tt> parameters. If the number of elements of the
lists are different then, an error occurs. The list
<tt>actual_list</tt> must be a list of non-flattened actual
parameters. For hrc structure it is not necessary to store the
flattening information that is implicit in the hierarchy.]

SideEffects        [In <tt>hrc_result</tt> the lists of formal and
actual parameter used to instatiate a module is changed.]

******************************************************************************/
static void make_params_hrc(node_ptr basename,
                            node_ptr actual_list,
                            node_ptr formal_list,
                            HrcNode_ptr hrc_result)
{
  nusmv_assert(HRC_NODE(NULL) != hrc_result);

  /* DO NOT CHANGE yylineno, now it points to declared instance basename */

  while (formal_list) {
    node_ptr actual_parameter;
    node_ptr formal_parameter_hrc;
    node_ptr formal_parameter_node;
    node_ptr actual_parameter_node;

    if (!actual_list) {
      if (basename != (node_ptr) NULL) {
        fprintf(nusmv_stderr, "While creating instance ");
        print_node(nusmv_stderr, basename);
        rpterr("too few actual parameters");
      }
      else {
        rpterr("module 'main' cannot have formal parameters");
      }
    }

    /* get the current actual and formal parameters */
    formal_parameter_hrc = car(formal_list);
    actual_parameter = car(actual_list);

    /* advance actual and formal lists */
    formal_list = cdr(formal_list);
    actual_list = cdr(actual_list);



    formal_parameter_node = cons(formal_parameter_hrc, Nil);
    HrcNode_add_formal_parameter(hrc_result, formal_parameter_node);

    /* When parameters will have a type Nil must be replaced */
    actual_parameter_node = cons(actual_parameter, Nil);
    HrcNode_add_actual_parameter(hrc_result, actual_parameter_node);


  }

  if (actual_list) rpterr("too many actual parameters");
}


/**Function********************************************************************

   Synopsis           [Instantiates all in the body of a module.]

   Description        [This function is responsible of the
   instantiation of the body of a module. The module definition
   (parameter and body) is <tt>mod_def</tt> and the module instance name
   <tt>mod_name</tt> are passed as arguments. First we instantiate the
   arguments of the given module. Then it loops over the module
   definition searching for defined symbols (i.e. those introduced by
   the keyword <tt>DEFINE</tt>) and inserts their definition in the
   <tt>symbol_hash</tt>. After this preliminary phase it loops again
   over module body in order to performs the other instantiation, and
   to extract all the information needed to compile the automaton,
   i.e. the list of processes, the TRANS statements, the INIT
   statements, ... and so on.

   NB: After parsing and creating the module hash table, the order of
   declarations is correct (not reversed). This function reverse the order
   of SPEC, LTLSPEC, PSLSPEC, INVARSPEC, COMPUTE, JUSTICE AND COMPATION
   ]


   SideEffects        []

   SeeAlso            [compile_instantiate_var compile_instantiate_vars]

******************************************************************************/
static void compile_instantiate(SymbTable_ptr st,
                                SymbLayer_ptr layer,
                                node_ptr mod_def,
                                node_ptr mod_name,
                                node_ptr actual,
                                node_ptr *assign,
                                FlatHierarchy_ptr result,
                                HrcNode_ptr hrc_result,
                                hash_ptr instances)
{
  node_ptr mod_body_decls;
  node_ptr mod_formal_args  = car(mod_def); /* Module formal parameters */
  node_ptr mod_body         = cdr(mod_def); /* Module body */

  /* creates local parameters */
  make_params(layer, mod_name, actual, mod_formal_args);

  /* We first instantiate all the definitions, in case they are
     constants used in the array declarations.
     loop over module declaration
  */
  for (mod_body_decls = mod_body; mod_body_decls != Nil;
       mod_body_decls = cdr(mod_body_decls)) {

    node_ptr cur_decl = car(mod_body_decls);

    switch (node_get_type(cur_decl)) {
    case DEFINE:
      {
        node_ptr define_iter;
        /* loop over DEFINE declaration */
        for (define_iter = car(cur_decl); define_iter != Nil;
             define_iter = cdr(define_iter)) {
          node_ptr cur_define = car(define_iter);

          node_ptr local_name = car(cur_define);
          node_ptr definition = cdr(cur_define);
          ResolveSymbol_ptr rs;
          node_ptr name;

          rs = SymbTable_resolve_symbol(st, local_name, mod_name);
          name = ResolveSymbol_get_resolved_name(rs);

          yylineno = node_get_lineno(define_iter);
          if (SymbLayer_can_declare_define(layer, name)) {
            /* If this is an array definition expand the definition
               just like array variables. Array defines are stored in
               parse-tree as normal defines, but they can be
               distinguished by ARRAY_DEF at top of the expression */
            if (ARRAY_DEF == node_get_type(definition)) {
              instantiate_array_define(st, layer, name, mod_name, definition);

              /* Inserts define array in hrc structure */
              if (HRC_NODE(NULL) != hrc_result) {
                /* Uses car(cur_define), the non-flattened name */
                node_ptr hrc_define = cons(car(cur_define), definition);

                /* Defines are visited in inversed order of
                   declaration, so the resulting list is in order */
                HrcNode_add_array_define(hrc_result, hrc_define);
              }

            }
            else {
              SymbLayer_declare_define(layer, name, mod_name, definition);

              if (HRC_NODE(NULL) != hrc_result) {
                /* Uses car(cur_define), the non-flattened name */
                node_ptr hrc_define = cons(car(cur_define), definition);

                /* Defines are visited in inversed order of
                   declaration, so the resulting list is in order */
                HrcNode_add_define(hrc_result, hrc_define);
              }
            }
          }
          else {
            yylineno = node_get_lineno(local_name); /* set correct line info */
            /* more precise error message */
            if (SymbTable_is_symbol_parameter(st, name)) error_shadowing(name);
            else error_redefining(name);
          }
        }/* loop on defines */
      }
      break;
    default: break;
    }
  } /* loop over module declarations */


  /* Now, we instantiate all the other elements of a module.
     loop again over module declaration
  */
  for (mod_body_decls = mod_body; mod_body_decls != Nil;
       mod_body_decls = cdr(mod_body_decls)) {
    node_ptr cur_decl = car(mod_body_decls);
    node_ptr tmp;
    yylineno = node_get_lineno(cur_decl);

    switch (node_get_type(cur_decl)) {
    case ISA:

      if (HRC_NODE(NULL) != hrc_result) {
        HrcNode_ptr root = get_hrc_root_node(hrc_result);
        /* Set a flag so we know that the HRC hierarchy is not usable */
        HrcNode_set_undef(root, (void*)~0);
        hrc_result = HRC_NODE(NULL);
      }
      compile_instantiate_by_name(st, layer, car(cur_decl), mod_name,
                                  Nil, assign, result, hrc_result, instances);
      break;

    case VAR:
      compile_instantiate_vars(st, layer, car(cur_decl), mod_name,
                               assign, result, hrc_result, instances);
      break;

    case FROZENVAR:
      set_variable_instantiation_to_frozen();
      compile_instantiate_vars(st, layer, car(cur_decl), mod_name,
                               assign, result, hrc_result, instances);
      set_variable_instantiation_to_state();
      break;

    case IVAR:
      set_variable_instantiation_to_input();
      compile_instantiate_vars(st, layer, car(cur_decl), mod_name,
                               assign, result, hrc_result, instances);
      set_variable_instantiation_to_state();
      break;

    case TRANS:
      tmp = find_node(AND, FlatHierarchy_get_trans(result),
                      find_node(CONTEXT, mod_name, car(cur_decl)));
      if (HRC_NODE(NULL) != hrc_result)
        HrcNode_add_trans_expr(hrc_result, car(cur_decl));
      FlatHierarchy_set_trans(result, tmp);
      break;

    case INIT:
      tmp = find_node(AND, FlatHierarchy_get_init(result),
                      find_node(CONTEXT, mod_name, car(cur_decl)));
      if (HRC_NODE(NULL) != hrc_result)
        HrcNode_add_init_expr(hrc_result, car(cur_decl));
      FlatHierarchy_set_init(result, tmp);
      break;

    case INVAR:
      tmp = find_node(AND, FlatHierarchy_get_invar(result),
                      find_node(CONTEXT, mod_name, car(cur_decl)));
      if (HRC_NODE(NULL) != hrc_result)
        HrcNode_add_invar_expr(hrc_result, car(cur_decl));
      FlatHierarchy_set_invar(result, tmp);
      break;

   /* ---------------------------------------------------------------------- */
   /* contexts of all kind of properties are 'flattened' here                */
    case SPEC:
      {
        node_ptr property_name = cdr(cur_decl);

        if (node_get_type(car(cur_decl)) == CONTEXT) {
          /* concatenates local context to the current module */
          node_ptr new_ctx = CompileFlatten_concat_contexts(mod_name,
                                                            caar(cur_decl));
          tmp = find_node(CONTEXT, new_ctx, cdr(car(cur_decl)));
        }
        else tmp = find_node(CONTEXT, mod_name, car(cur_decl));

        /* Support for property names */
        if (Nil != property_name) {
          property_name = CompileFlatten_concat_contexts(mod_name,
                                                         property_name);
          if (!FlatHierarchy_add_property_name(result, property_name)){
            error_redefining(property_name);
          }
        }

        tmp = find_node(SPEC, tmp, property_name);
        tmp = cons(tmp, FlatHierarchy_get_spec(result));
        FlatHierarchy_set_spec(result, tmp);
        if (HRC_NODE(NULL) != hrc_result)
          HrcNode_add_ctl_property_expr(hrc_result, cur_decl);
      }
      break;

    case LTLSPEC:
      {
        node_ptr property_name = cdr(cur_decl);

        if (node_get_type(car(cur_decl)) == CONTEXT) {
          /* concatenates local context to the current module */
          node_ptr new_ctx = CompileFlatten_concat_contexts(mod_name,
                                                            caar(cur_decl));
          tmp = find_node(CONTEXT, new_ctx, cdr(car(cur_decl)));
        }
        else tmp = find_node(CONTEXT, mod_name, car(cur_decl));

        /* Support for property names */
        if (Nil != property_name) {
          property_name = CompileFlatten_concat_contexts(mod_name,
                                                         property_name);
          if (!FlatHierarchy_add_property_name(result, property_name)){
            error_redefining(property_name);
          }
        }
        tmp = find_node(LTLSPEC, tmp, property_name);

        tmp = cons(tmp, FlatHierarchy_get_ltlspec(result));
        FlatHierarchy_set_ltlspec(result, tmp);

        if (HRC_NODE(NULL) != hrc_result)
          HrcNode_add_ltl_property_expr(hrc_result, cur_decl);
      }
      break;

    case PSLSPEC:
      {
        node_ptr property_name = cdr(cur_decl);

        if (node_get_type(car(cur_decl)) == CONTEXT) {
          /* concatenates local context to the current module */
          node_ptr new_ctx = CompileFlatten_concat_contexts(mod_name,
                                                            caar(cur_decl));
          tmp = PslNode_new_context(new_ctx, cdr(car(cur_decl)));
        }
        else tmp = find_node(CONTEXT, mod_name, car(cur_decl));

        /* Support for property names */
        if (Nil != property_name) {
          property_name = CompileFlatten_concat_contexts(mod_name,
                                                         property_name);
          if (!FlatHierarchy_add_property_name(result, property_name)){
            error_redefining(property_name);
          }
        }
        tmp = find_node(PSLSPEC, tmp, property_name);

        tmp = cons(tmp, FlatHierarchy_get_pslspec(result));
        FlatHierarchy_set_pslspec(result, tmp);

        if (HRC_NODE(NULL) != hrc_result)
          HrcNode_add_psl_property_expr(hrc_result, cur_decl);
      }
      break;

    case INVARSPEC:
      {
        node_ptr property_name = cdr(cur_decl);

        if (node_get_type(car(cur_decl)) == CONTEXT) {
          /* concatenates local context to the current module */
          node_ptr new_ctx = CompileFlatten_concat_contexts(mod_name,
                                                            caar(cur_decl));
          tmp = find_node(CONTEXT, new_ctx, cdr(car(cur_decl)));
        }
        else tmp = find_node(CONTEXT, mod_name, car(cur_decl));

        /*  Support for property names */
        if (Nil != property_name) {
          property_name = CompileFlatten_concat_contexts(mod_name,
                                                         property_name);
          if (!FlatHierarchy_add_property_name(result, property_name)){
            error_redefining(property_name);
          }
        }
        tmp = find_node(INVARSPEC, tmp, property_name);

        tmp = cons(tmp, FlatHierarchy_get_invarspec(result));
        FlatHierarchy_set_invarspec(result, tmp);

        if (HRC_NODE(NULL) != hrc_result)
          HrcNode_add_invar_property_expr(hrc_result, cur_decl);
      }
      break;

    case COMPUTE:
      {
        node_ptr property_name = cdr(cur_decl);

        if (node_get_type(car(cur_decl)) == CONTEXT) {
          /* concatenates local context to the current module */
          node_ptr new_ctx = CompileFlatten_concat_contexts(mod_name,
                                                            caar(cur_decl));
          tmp = find_node(CONTEXT, new_ctx, cdr(car(cur_decl)));
        }
        else tmp = find_node(CONTEXT, mod_name, car(cur_decl));

        /*  Support for property names */
        if (Nil != property_name) {
          property_name = CompileFlatten_concat_contexts(mod_name,
                                                         property_name);
          if (!FlatHierarchy_add_property_name(result, property_name)){
            error_redefining(property_name);
          }
        }
        tmp = find_node(COMPUTE, tmp, property_name);

        tmp = cons(tmp, FlatHierarchy_get_compute(result));
        FlatHierarchy_set_compute(result, tmp);

        if (HRC_NODE(NULL) != hrc_result)
          HrcNode_add_compute_property_expr(hrc_result, cur_decl);
      }
      break;
   /* ---------------------------------------------------------------------- */

    case JUSTICE:
      tmp = cons(find_node(CONTEXT, mod_name, car(cur_decl)),
                 FlatHierarchy_get_justice(result));
      FlatHierarchy_set_justice(result, tmp);

      if (HRC_NODE(NULL) != hrc_result)
        HrcNode_add_justice_expr(hrc_result, car(cur_decl));
      break;

    case COMPASSION:
      tmp = cons(cons(find_node(CONTEXT, mod_name, car(car(cur_decl))),
                      find_node(CONTEXT, mod_name, cdr(car(cur_decl)))),
                 FlatHierarchy_get_compassion(result));
      FlatHierarchy_set_compassion(result, tmp);

      if (HRC_NODE(NULL) != hrc_result) {
        node_ptr hrc_compassion =
          cons(car(car(cur_decl)), cdr(car(cur_decl)));

        HrcNode_add_compassion_expr(hrc_result, hrc_compassion);
      }

      break;

    case ASSIGN:
      {
        /* an assign may be void */
        if (car(cur_decl) != Nil) {
          *assign = find_node(AND, *assign,
                              find_node(CONTEXT, mod_name, car(cur_decl)));
        }

        if (HRC_NODE(NULL) != hrc_result) {
          compile_insert_assign_hrc(hrc_result, cur_decl);
        }
      }
      break;
    case DEFINE: break; /* already dealt with */

    case CONSTANTS:
      /* declares the contained constants: */
      flatten_declare_constants_within_list(st,
                                            layer,
                                            reverse_ns(car(cur_decl)));

      if (HRC_NODE(NULL) != hrc_result) {
        HrcNode_add_constants(hrc_result, car(cur_decl));
      }

      break;

    case PRED:
      FlatHierarchy_add_pred(result, cur_decl);
      break;

    case MIRROR:
      FlatHierarchy_add_mirror(result, cur_decl);
      break;

    default: error_unreachable_code(); /* unknown kind of declaration */
    }
  } /* loop over module declarations */

}


/**Function********************************************************************

   Synopsis           [Starts the flattening from a given point in the
   module hierarchy.]

   Description        [<tt>module_name</tt> is the name of the module being
   instantiated. The name of the module instance
   is <tt>instance_name</tt>. First checks if the module exists. Then it checks
   if the module is recursively defined, and if the case an error is
   printed out. If these checks are passed, then it proceeds in the
   instantiation of the body of the module.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void compile_instantiate_by_name(SymbTable_ptr st,
                                        SymbLayer_ptr layer,
                                        node_ptr module_name,
                                        node_ptr instance_name,
                                        node_ptr actual,
                                        node_ptr *assign,
                                        FlatHierarchy_ptr result,
                                        HrcNode_ptr hrc_result,
                                        hash_ptr instances)
{
  node_ptr s;
  node_ptr mod_name = find_atom(module_name);         /* find module name */
  /* find module definition */
  node_ptr mod_def  = lookup_module_hash(mod_name);

  if (mod_def == (node_ptr) NULL) {
    /* The module is undefined */
    yylineno = node_get_lineno(module_name);
    error_undefined(module_name);
  }

  /* DO NOT CHANGE yylineno, now it points to declared instance
     instance_name */

  /* scans module_stack in order to find if there are recursively
     defined modules */
  s = module_stack;
  while (s != Nil) {
    if (car(s) == mod_name) {
      rpterr("module \"%s\" is recursively defined",
             get_text((string_ptr)car(module_name)));
    }
    s = cdr(s);
  }

  /* append current module to module_stack */
  module_stack = cons(mod_name, module_stack);

  compile_instantiate(st, layer, mod_def, instance_name, actual,
                      assign, result, hrc_result, instances);

  /* elimination of current module form module_stack */
  s = cdr(module_stack);
  free_node(module_stack);
  module_stack = s;
}


/**Function********************************************************************

   Synopsis           [Recursive function for flattenig a sexp.]

   Description        [

   DOCUMENTATION ABOUT ARRAY:

      In NuSMV ARRAY has 2 meanings, it can be a part of identifier
   (which we call identifier-with-brackets) or a part of
   expression. For example, VAR v[5] : boolean; declares a new
   identifier-with-brackets v[5] where [5] is a part of
   identifier. Thus v[d], where d is a define equal to 5, is not a
   valid identifier as well as v[4+1] or v, whereas v[5] is valid.

   For "VAR v : array 1..5 of boolean;" v[5] is identifier (array
   elements are declared in symbol table) v[d] is not,
   but both are valid expressions.

   This difference is important for code, e.g.
     DEFINE d := v;
     INVARSPEC d[5];
   If v[5] is declared as individual identifier this code is invalid
   because v is not declared whereas if v is an array the code becomes
   valid.

   Flattener additionally makes every ARRAY-expression normalized.
   For example, d[i+1] is changed to
   case i+1=0 : v[0]; i+1=1 : v[1]; ... FAILURE; esac.
   Such a way every v[N] become a legal identifier wrt symbol table.
   Note that above normalization is done independent if defines are set
   to be expanded or not.

   NOTE: arrays of modules are not supported. Thus ARRAY before DOT
   can be legal only through identifier-with-brackets declaration, e.g.
   for v[3].b to be valid it is necessary to declare v[3] as module instance.
   ]

   SideEffects        []

   SeeAlso            [Compile_FlattenSexp Compile_FlattenSexpExpandDefine]

******************************************************************************/
static node_ptr
compileFlattenSexpRecur(const SymbTable_ptr symb_table,
                        node_ptr sexp, node_ptr context)
{

  /* currently this function applies find_atom to the constants and IDs
     and new_node to operations nodes. If this approach changes
     then internal function ltl_rewrite_input has to be changed as well.
     NB: this is not a warning but a note. warning is used to make the message
     look important. */

  node_ptr result = Nil;
  int temp = yylineno;

  if (sexp == Nil) return sexp;

  yylineno = node_get_lineno(sexp);

  switch (node_get_type(sexp)) {
    /* base cases for which no flattening necessary */
  case FAILURE:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    result = find_atom(sexp);
    break;

  case SWCONST:
  case UWCONST: /* word constants are removed from the expressions,
                   as they are handled only at flattening time */
    {
      node_ptr value = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr width = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      int width_int;
      int value_int;
      WordNumber_ptr value_word;

      WordNumberValue tmp;
      int type = node_get_type(sexp);

      value = CompileFlatten_resolve_number(symb_table, value, context);
      width = CompileFlatten_resolve_number(symb_table, width, context);

      /*  --- process the width: it can be an integer or word number
          in range [0,max-allowed-width] */
      if ((node_ptr) NULL == width) {
        rpterr("width specifier of swconst/uwconst operator is "
               "not a constant");
      }

      switch (node_get_type(width)) {
      case NUMBER:
        width_int = node_get_int(width);
        break;

      case NUMBER_UNSIGNED_WORD:
        tmp = WordNumber_get_unsigned_value(WORD_NUMBER(car(width)));
        width_int = tmp;
        if (tmp != width_int) {
          rpterr("width specifier of swconst/uwconst operator is "
                 "not representable as int");
        }
        break;

      case NUMBER_SIGNED_WORD:
        tmp = WordNumber_get_signed_value(WORD_NUMBER(car(width)));
        width_int = tmp;
        if (tmp != width_int) {
          rpterr("width specifier of swconst/uwconst operator is "
                 "not representable as int");
        }
        break;

      default: /* error */
        rpterr("width specifier of swconst/uwconst operator is "
               "not an integer or word constant");
      }

      if (width_int <= 0 || width_int > WordNumber_max_width()) {
        rpterr("width specifier is out of range [0, %i]",
               WordNumber_max_width());
      }

      /*  --- process the value: it can be only integer and
          has to be representable with given width */
      if ((node_ptr) NULL == value  || NUMBER != node_get_type(value)) {
        rpterr("value specifier of swconst/uwconst operator is not "
               "an integer constant");
      }

      value_int = node_get_int(value);

      /* two shifts are done because shift by the full width isn't allowed in
         C. If value is positive, an extra bit of width is needed to avoid
         overflow. */
      if ((value_int > 0 &&
           ((UWCONST == type && value_int >> (width_int-1) >> 1 != 0) ||
            (SWCONST == type && value_int >> (width_int-2) >> 1 != 0))) ||
          (value_int < 0 && value_int >> (width_int-1) != -1)) {
        rpterr("value specifier of swconst/uwconst operator is not "
               "representable with provided width");
      }

      value_word = (value_int >= 0)
        ? WordNumber_from_integer((WordNumberValue)value_int, width_int)
        : WordNumber_from_signed_integer((WordNumberValue)value_int,
                                         width_int);

      nusmv_assert(WORD_NUMBER(NULL) != value_word);

      result = find_node((UWCONST == type)
                         ? NUMBER_UNSIGNED_WORD : NUMBER_SIGNED_WORD,
                         NODE_PTR(value_word) , Nil);
      break;
    }

  case ATTIME:
    {
      node_ptr left  = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr right = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      result = find_node(ATTIME, left, right);
      break;
    }

  case BIT:
    result = sexp;
    break;

  case ATOM:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, sexp, context);

      name = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_error(rs)) ResolveSymbol_throw_error(rs);

      if (ResolveSymbol_is_constant(rs)) {
        result = name;
        break;
      }

      if (ResolveSymbol_is_parameter(rs)) {
        node_ptr param;


        param = SymbTable_get_flatten_actual_parameter(symb_table, name);
        /* The result of the flattening is then flattening of parameters */
        result = compileFlattenSexpRecur(symb_table, param, context);
        break;
      }

      /* It can be a defined symbol, a running condition or a variable */
      result = Flatten_GetDefinition(symb_table, name);
      break;
    }

  case SELF:
    fprintf(nusmv_stderr,
            "compileFlattenSexpRecur: invalid usage of identifier \"self\"\n");

    error_reset_and_exit(1);
    break;

  case DOT:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, sexp, context);
      name = ResolveSymbol_get_resolved_name(rs);

      result = Flatten_GetDefinition(symb_table, name);
      break;
    }

  case ARRAY:
    {
      boolean is_next;
      node_ptr array, index, tmp;
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, sexp, context);
      name = ResolveSymbol_get_resolved_name(rs);

      /* ! See description of compileFlattenSexpRecur for
         information about how ARRAY are implemented !
      */

      /* at first resolve the name and it is already declared
         then return the results */
      if (Nil != name &&
          ResolveSymbol_is_defined(rs)) {
        result = Flatten_GetDefinition(symb_table, name);
        break;
      }

      /* this is array-expression, not identifier-with-brackets */
      array = car(sexp);
      index = cdr(sexp);
      is_next = false;

      if (NEXT == node_get_type(array)) { /* exp of form "next(m)[i]" */
        is_next = true;
        array = car(array);
      }

      /* Resolve the array LHS which can be arbitrary expression */
      array = compileFlattenSexpRecur(symb_table, array, context);

      /* get rid of defines */
      while (SymbTable_is_symbol_define(symb_table, array)) {
        node_ptr body = SymbTable_get_define_body(symb_table, array);
        node_ptr ctx = SymbTable_get_define_context(symb_table, array);
        /* expression is arbitrary => flatten it by standard procedure */
        array = compileFlattenSexpRecur(symb_table, body, ctx);
      }

      /* after flattening a new NEXT may appear (e.g. from defines) */
      if (NEXT == node_get_type(array)) {
        nusmv_assert(!is_next); /*double next*/
        is_next = true;
        array = car(array);
      }

      /* index is arbitrary exp => flatten it by standard procedure
         and then try to get constant value if it is possible */
      index = compileFlattenSexpRecur(symb_table, index, context);

      tmp = CompileFlatten_resolve_number(symb_table, index, context);
      if (tmp != Nil) index = tmp; /* The current index is constant */

      if (Nil == index) {

        /* Even if index was not resolved, the expression could be valid
           for example in PSL forall

          return find_node(ARRAY, array, cdr(n));
        */
        /* The index has to be solved */
        error_unreachable_code();
      }

      /* convert word constant to integer constant */
      if (NUMBER_SIGNED_WORD == node_get_type(index) ||
          NUMBER_UNSIGNED_WORD == node_get_type(index)) {
        WordNumberValue val
          = WordNumber_get_unsigned_value((WordNumber_ptr) car(index));
        index = find_node(NUMBER, NODE_FROM_INT((int)val), Nil);

        /* if below assertion is violated it means that int is not enough
           to represent indexes of arrays. Representation of nodes
           have to be changed then (as NUMBER keep numbers in int). */
        nusmv_assert(((int)val) == val);
      }

      if (node_get_type(array) == IFTHENELSE ||
          node_get_type(array) == CASE) {
        /* If left child of ARRAY is if-then-else then push index-access
           down to leaves of if-then-else and resolve identifiers there.
           Note that both array and index are already resolved thus
           context is not required.
        */
        result = push_array_index_down(array, index, is_next, symb_table);
        break;
      }

      /* if not if-then-else then only identifier is possible on LHS.
         Actually, since type checking is not done yet, also
         invalid expression is possible */

      if (NUMBER != node_get_type(index)) {
        /* Index is an (dynamic) expression => convert
           the index-access to if-then-else, e.g.
           a[i] is converted to
           case i = 0 : a[0];
                i = 1 : a[1];
                ...
                i = N : a[N];
                failure;
           esac;
        */
        result = construct_array_multiplexer(array, index, is_next,
                                              symb_table);
        break;
      }

      /* we have a constant index => check that it is in the range */
      {
        SymbType_ptr type = TypeChecker_get_expression_type(
                                       SymbTable_get_type_checker(symb_table),
                                       array, Nil);
        int lower_bound, upper_bound, val;

        if (!SymbType_is_array(type)) {
          error_lhs_of_index_is_not_array();
        }
        lower_bound = SymbType_get_array_lower_bound(type);
        upper_bound = SymbType_get_array_upper_bound(type);
        val = node_get_int(index);
        if (val < lower_bound || val > upper_bound) {
          error_array_out_of_bounds(val, lower_bound, upper_bound);
        }
      }

      /* index is a constant => create the array expression and
         expand if required.*/
      array = find_node(ARRAY, array, index);
      if (definition_mode_is_expand()) {
        array = Flatten_GetDefinition(symb_table, array);
      }
      /* add the final next if required */
      if (is_next) array = find_node(NEXT, array, Nil);
      result = array;
      break;
    }

  case SPEC:
  case INVARSPEC:
  case LTLSPEC:
  case PSLSPEC:
  case COMPUTE:
    {
      node_ptr ctx = car(sexp);
      result = compileFlattenSexpRecur(symb_table, cdr(ctx), car(ctx));
    }
    break;

  case CONTEXT:
    /* (CONTEXT (cxt . expr)) */
    result = compileFlattenSexpRecur(symb_table, cdr(sexp), car(sexp));
    break;

  case NEXT:
    {
      node_ptr body = compileFlattenSexpRecur(symb_table, car(sexp), context);
      result = new_node(node_get_type(sexp), body, Nil);
      break;
    }

    /* Unary operators */
  case NOT:
  case CAST_WORD1:
  case CAST_SIGNED:
  case CAST_UNSIGNED:
  case UMINUS:
    {
      node_ptr body = compileFlattenSexpRecur(symb_table, car(sexp), context);
      result = new_node(node_get_type(sexp), body, Nil);
      break;
    }

  case CAST_BOOL:
    {
      node_ptr body = compileFlattenSexpRecur(symb_table, car(sexp), context);
      SymbType_ptr type =
        TypeChecker_get_expression_type(SymbTable_get_type_checker(symb_table),
                                        body, Nil);

      /* No cast needed */
      if (SymbType_is_boolean(type)) {
        result = body;
      }
      else if (SymbType_is_integer(type) ||
               SymbType_is_real(type) ||
               SymbType_is_pure_int_enum(type)) {
        /* int = 0 ? FALSE : TRUE */
        node_ptr zero = find_node(NUMBER, NODE_FROM_INT(0), Nil);
        result = new_node(CASE, new_node(COLON, new_node(EQUAL, body, zero),
                                         Expr_false()), Expr_true());
      }
      else if (SymbType_is_word_1(type)) {
        /* w1 = 0ud1_0 ? FALSE : TRUE */
        node_ptr w0ud1 = find_node(NUMBER_UNSIGNED_WORD,
                                   NODE_PTR(WordNumber_from_integer(0,1)), Nil);
        result = new_node(CASE, new_node(COLON, new_node(EQUAL, body, w0ud1),
                                         Expr_false()), Expr_true());
      }
      else {
        error_invalid_bool_cast(body);
      }

      break;
    }

  case CAST_TOINT:
    {
      node_ptr body;
      SymbType_ptr type;

      body = compileFlattenSexpRecur(symb_table, car(sexp), context);

      type = TypeChecker_get_expression_type(SymbTable_get_type_checker(symb_table),
                                             body, Nil);
      /* No cast needed */
      if (SymbType_is_integer(type) ||
          SymbType_is_real(type) ||
          SymbType_is_pure_int_enum(type)) {
        result = body;
      }
      else if (SymbType_is_boolean(type)) {
        node_ptr one = find_node(NUMBER, NODE_FROM_INT(1), Nil);
        node_ptr zero = find_node(NUMBER, NODE_FROM_INT(0), Nil);

        result = new_node(CASE, new_node(COLON, body, one), zero);
      }
      else if (SymbType_is_word(type)) {
        result = compile_flatten_rewrite_word_toint_cast(body, type);
      }
      else {
        error_invalid_toint_cast(body);
      }
      break;
    }

  case COUNT:
    {
      SymbType_ptr type;
      TypeChecker_ptr tc = SymbTable_get_type_checker(symb_table);
      node_ptr list = car(sexp);
      node_ptr new_expr = Nil;
      nusmv_assert(Nil != list);

      /* Process the first element. There must be at least one
         parameter. */
      do {
        node_ptr cur_expr = car(list);
        node_ptr toint;


        /* First of all, check if this is a boolean expression */
        type = TypeChecker_get_expression_type(tc, cur_expr, context);
        if (!SymbType_is_boolean(type)) {
          error_invalid_count_operator(cur_expr);
        }

        /* Rewrite the expression as toint(expr) */
        toint = compileFlattenSexpRecur(symb_table,
                                        new_node(CAST_TOINT, cur_expr, Nil),
                                        context);

        if (Nil == new_expr) {
          new_expr = toint;
        }
        else {
          new_expr = new_node(PLUS, toint, new_expr);
        }

        list = cdr(list);
      } while (Nil != list);

      result = new_expr;
    }
    break;

    /* binary operators */
  case CONS:
  case AND:
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
  case LSHIFT:
  case RSHIFT:
  case LROTATE:
  case RROTATE:
  case LT:
  case GT:
  case LE:
  case GE:
  case UNION:
  case SETIN:
  case EQUAL:
  case NOTEQUAL:
  case CONCATENATION:
  case COLON: /* can be part of CASE or BIT_SELECTION only */
              /* EDIT: it is also part of IFTHENELSE*/
  case CASE:
  case IFTHENELSE:
  case WAREAD:
  case WAWRITE:
    {
      node_ptr left  = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr right = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      result = new_node(node_get_type(sexp), left, right);
      break;
    }

  case WRESIZE:
    {
      node_ptr left  = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr right = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      { /* just check */
        node_ptr value = CompileFlatten_resolve_number(symb_table,
                                                       right,
                                                       context);

        if ((Nil == value) || (NUMBER != node_get_type(value))) {
          error_not_constant_resize_width(cdr(sexp));
        }
      }
      result = new_node(node_get_type(sexp), left, right);
      break;
    }

  case WSIZEOF:
    {
      node_ptr body = compileFlattenSexpRecur(symb_table, car(sexp), context);
      SymbType_ptr type =
        TypeChecker_get_expression_type(SymbTable_get_type_checker(symb_table),
                                        body, Nil);

      if (SymbType_is_error(type)) { error_not_word_wsizeof(car(sexp)); }

      {
        int width = SymbType_get_word_width(type);
        nusmv_assert(0 < width);

        result = find_node(NUMBER, NODE_FROM_INT(width), Nil);
        break;
      }
    }

  case EXTEND:
    {
      node_ptr base = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr delta = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      {
        node_ptr value = CompileFlatten_resolve_number(symb_table,
                                                       delta,
                                                       context);

        if ((Nil == value) || NUMBER != node_get_type(value)) {

          error_not_constant_extend_width(cdr(sexp));
        }
      }
      result = new_node(EXTEND, base, delta);
      break;
    }

  case BIT_SELECTION:
    {
      node_ptr base = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr bits = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      result = new_node(BIT_SELECTION, base, bits);
      break;
    }

  case TWODOTS:
    {
      /* We don't need to expand it, eval did it */
      result = sexp;
      break;
    }

    /* CTL Unary operators */
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
    {
      node_ptr body = compileFlattenSexpRecur(symb_table, car(sexp), context);

      result = new_node(node_get_type(sexp), body, Nil);
      break;
    }

    /* CTL bynary operators */
  case EU:
  case AU:
    {
      node_ptr left  = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr right = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      result = new_node(node_get_type(sexp), left, right);
      break;
    }

    /* CTL bounded Temporal Operators */
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU:
    {
      node_ptr body  = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr range = cdr(sexp);

      /* checks the range: */
      if (! Utils_check_subrange_not_negative(range) ) {
        error_invalid_subrange(range);
      }

      result = new_node(node_get_type(sexp), body, range);
      break;
    }

    /* LTL unary temporal operators */
  case OP_NEXT:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
    {
      node_ptr body  = compileFlattenSexpRecur(symb_table, car(sexp), context);
      result = new_node(node_get_type(sexp), body, Nil);
      break;
    }

    /* LTL binary temporal operators */
  case UNTIL:
  case RELEASES:
  case SINCE:
  case TRIGGERED:
    {
      node_ptr left   = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr right  = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      result = new_node(node_get_type(sexp), left, right);
      break;
    }

    /* MIN and MAX operators */
  case MINU:
  case MAXU:
    {
      node_ptr left   = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr right  = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      result = new_node(node_get_type(sexp), left, right);
      break;
    }
  case EQDEF:
    {
      node_ptr left  = car(sexp);
      node_ptr right = cdr(sexp) ;
      node_ptr res_left, res_right;
      node_ptr name;

      switch (node_get_type(left)) {
      case SMALLINIT:
      case NEXT:
        /* we are dealing with init(x) := init_expr or next(x) := next_expr */
        name = compileFlattenSexpRecur(symb_table, car(left), context);
        res_left = new_node(node_get_type(left), name, Nil);
        res_right = compileFlattenSexpRecur(symb_table, right, context);
        break;

      default:
        /* we are dealing with x := simple_expr */
        name = res_left = compileFlattenSexpRecur(symb_table, left, context);
        res_right = compileFlattenSexpRecur(symb_table, right, context);
        break;
      }


      if (node_get_type(name) == IFTHENELSE ||
          node_get_type(name) == CASE) {
        /* Note about array defines.
           Dynamic indexes are not allowed in assignment.
           Thus no if-then-else expression is possible here */
        rpterr("Expressions not allowed in array subscripts on "
               "left hand side of assignments");
      }
      if (SymbTable_is_symbol_variable_array(symb_table, name)) {
        rpterr("It is not possible to assign a whole array");
      }
      if (node_get_type(name) == BIT_SELECTION) {
        error_bit_selection_assignment_not_supported(left);
      }
      /* name is expected to be a variable */
      if (!SymbTable_is_symbol_var(symb_table, name)) {
        error_assign_expected_var(sexp);
      }

      result = new_node(EQDEF, res_left, res_right);
      break;
    }

  case ARRAY_DEF:
    {
      node_ptr left   = compileFlattenSexpRecur(symb_table, car(sexp), context);
      node_ptr right  = compileFlattenSexpRecur(symb_table, cdr(sexp), context);

      result = new_node(ARRAY_DEF, left, right);
      break;
    }

  case NFUNCTION:
    {
      node_ptr params = compileFlattenSexpRecur(symb_table, cdr(sexp), context);
      result = new_node(NFUNCTION, car(sexp), params);
    }
    break;

  default:
    fprintf(nusmv_stderr,
            "compileFlattenSexpRecur: undefined node type (%d)\n",
            node_get_type(sexp));
    error_reset_and_exit(1);
  }

  nusmv_assert(result != Nil);
  yylineno = temp;
  return result;
}


/**Function********************************************************************

   Synopsis           [Recursive definition of compileFlattenProcess]

   Description        [Recursive definition of compileFlattenProcess.
   If running is Nil there are no processes => no need to create
   data structure with CASEs (for next-assignments).]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void
compileFlattenProcessRecur(const SymbTable_ptr symb_table,
                           node_ptr assign, node_ptr context, node_ptr running,
                           FlatHierarchy_ptr flatHierarchy)
{
  if (assign == Nil) return;
  yylineno = node_get_lineno(assign);
  switch (node_get_type(assign)) {
  case CONS:
  case AND:
    compileFlattenProcessRecur(symb_table, car(assign), context,
                               running, flatHierarchy);
    compileFlattenProcessRecur(symb_table, cdr(assign), context,
                               running, flatHierarchy);
    break;

  case CONTEXT:
    compileFlattenProcessRecur(symb_table, cdr(assign), car(assign),
                               running, flatHierarchy);
    break;

  case EQDEF:
    {
      node_ptr vname, lhsa, stored;
      node_ptr left  = car(assign);
      node_ptr right = cdr(assign);
      ResolveSymbol_ptr rs;

      switch (node_get_type(left)) {
      case SMALLINIT: /* init assignement */ {
        rs = SymbTable_resolve_symbol(symb_table, car(left), context);
        vname = ResolveSymbol_get_resolved_name(rs);
        lhsa = find_node(node_get_type(left), vname, Nil);
        stored = FlatHierarchy_lookup_assign(flatHierarchy, lhsa);

        if (Nil != stored) error_reassigning(vname);
      }
        break;

      case NEXT: /* next assignement */
        {
          rs = SymbTable_resolve_symbol(symb_table, car(left), context);
          vname = ResolveSymbol_get_resolved_name(rs);
          lhsa = find_node(node_get_type(left), vname, Nil);
          stored = FlatHierarchy_lookup_assign(flatHierarchy, lhsa);

          /* there are processes => create CASE with "running" */
          if (Nil != proc_selector_internal_vname) {
            /* create default value for assignment, i.e. var name  */
            if (Nil == stored) stored = vname;
            /* create a CASE with running guard */
            right = new_node(CASE, new_node(COLON, running, right), stored);
          }
          else { /* no processes => no CASE things */
            if (Nil != stored) error_reassigning(vname);
          }
        }
        break;

      default:
        /* Invariant assignment */
        {
          rs = SymbTable_resolve_symbol(symb_table, left, context);
          vname = lhsa = ResolveSymbol_get_resolved_name(rs);
          stored = FlatHierarchy_lookup_assign(flatHierarchy, lhsa);

          if (Nil != stored)  error_reassigning(vname);
        }
      }
      FlatHierarchy_insert_assign(flatHierarchy, lhsa, right);


      break;
    } /* outer switch case EQDEF */

  default:
    internal_error("compileFlattenProcessRecur: type = %d",
                   node_get_type(assign));
  } /* outer switch case */

}


/**Function********************************************************************

   Synopsis           [Creates the internal process selector variable, within
   the given layer]

   Description        [Creates an input variable to denote
   the internal process selector, and the defines to denote
   the corresponding 'running' symbols.

   'process_name_list' is a list of existing processes names. If the list
   contains just one element ("main") no variables and defines are
   declared (no need). This happens if there is no "process" modules or
   the modules were flattened (which also removes "process" things).

   NB for developers: the internal process selector variable is by
   default positioned at the top of the ordering. It is attached to
   <tt>input_variables</tt> and <tt>all_variables</tt> too. ]

   SideEffects        [<tt>input_variables</tt> and
   <tt>all_variables</tt> are affected.]

   SeeAlso            []

******************************************************************************/
static void
create_process_symbolic_variables(SymbTable_ptr symb_table,
                                  SymbLayer_ptr layer,
                                  node_ptr process_name_list)
{
  /* the list of process always contain one element */
  nusmv_assert(CONS == node_get_type(process_name_list));

  /* there is just one module (main). Therefore, nothing should be done */
  if (Nil == cdr(process_name_list)) {
    /* during flattening "main" is denoted by Nil */
    nusmv_assert(Nil == car(process_name_list));

    return;
    /* Note that the symbols "_process_selector_" or "running" may be
       already defined. This happens, for example, if flattened module
       is read. But there is no need to care about it. If there are
       no processes then user can define its one _process_selector_ or
       running as usual symbols.
    */
  }

  /* -- There are several process -- */
  warning_processes_deprecated();

  /* initialise the global variable with "process_selector" name */
  proc_selector_internal_vname =
    find_node(DOT, Nil, sym_intern(PROCESS_SELECTOR_VAR_NAME));

  {
    /* internally "main" is denoted by Nil. change now Nil to "main". */
    node_ptr l = process_name_list;
    while (Nil != l && Nil != car(l)) l = cdr(l);

    /* there should always be a Nil element ("main" module)*/
    nusmv_assert(Nil != l);
    setcar(l, sym_intern("main"));
  }

  /* check that a user did not create its own  _process_selector_ */
  if (SymbTable_is_symbol_declared(symb_table, proc_selector_internal_vname)) {
    error_redefining_operational_symbol(proc_selector_internal_vname);
  }

  /* declare the "process-selector" symbol with a proper values */
  {
    SymbType_ptr symbolicType;

    flatten_declare_constants_within_list(symb_table, layer, process_name_list);
    symbolicType = SymbType_create(SYMB_TYPE_ENUM, process_name_list);
    SymbLayer_declare_input_var(layer,
                                proc_selector_internal_vname,
                                symbolicType);
  }


  /* Declare DEFINES representing "running"s symbols */
  {
    node_ptr main_atom = sym_intern("main");
    node_ptr running_atom = sym_intern(RUNNING_SYMBOL);
    node_ptr iter;

    for (iter = process_name_list; iter != Nil; iter = cdr(iter)) {
      node_ptr module_name, def_name, def_body;
      ResolveSymbol_ptr rs;

      module_name = car(iter);

      if (module_name == main_atom) {
        /* internally main is represented as Nil */
        rs = SymbTable_resolve_symbol(symb_table, running_atom, Nil);
      }
      else {
        rs = SymbTable_resolve_symbol(symb_table, running_atom, module_name);
      }

      def_name = ResolveSymbol_get_resolved_name(rs);

      /* check that the symbol has not been already defined */
      if (ResolveSymbol_is_defined(rs)) {
        error_redefining_operational_symbol(def_name);
      }

      /* creating the body of DEFINE: _process_selector = Pi */
      def_body = find_node(EQUAL, proc_selector_internal_vname, module_name);

      /* The flatten hash has to be filled with the flattened
         body of the newly defined symbol.
      */
      insert_flatten_def_hash(def_name,
                              Compile_FlattenSexp(symb_table, def_body, Nil));

      /* declare the define: */
      SymbLayer_declare_define(layer, def_name, Nil /*context*/, def_body);
    } /* for */
  }
}



/**Function********************************************************************

   Synopsis   [Create array multiplexer in order to get rid of dynamic
   indexes.]

   Description [This function takes index-access expression
   with dynamic index and returns if-then-else expression
   with all indexes are constants
   E.g.:
      a[i]
   is converted to
      case i = min-index : a [0];
           i = min-index+1 : a [1];
           ...
           i = max-index : a [max-index];
           failure;
       esac;

   Precondition: array is allowed to be an array variable or
   array define only.

   The array and its index have to be resolved already, i.e.
   context is not required.

   Flat is_array_next signals that array has to be
   wrapped in NEXT, whereas index is not.

   The minimal and maximal indexes are obtained from the type
   of the array.
   ]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static node_ptr construct_array_multiplexer(node_ptr array, node_ptr index,
                                             boolean is_array_next,
                                             SymbTable_ptr symb_table)
{
  int lower_bound, upper_bound;

  /* -- Find the lower and upper bound of the array.
     First, check if the id is an array variable */
  if (SymbTable_is_symbol_variable_array(symb_table, array)) {
    SymbType_ptr type = SymbTable_get_variable_array_type(symb_table, array);

    /* This symbol has to be an array variable of array type */
    nusmv_assert((SymbType_ptr)NULL != type);
    nusmv_assert(SymbType_is_array(type));

    lower_bound = SymbType_get_array_lower_bound(type);
    upper_bound = SymbType_get_array_upper_bound(type);
  }
  /* array may be an array define */
  else if (SymbTable_is_symbol_array_define(symb_table, array)) {
    node_ptr body = SymbTable_get_array_define_body(symb_table, array);

    lower_bound = 0;
    upper_bound = llength(car(body)) - 1;

    nusmv_assert(ARRAY_DEF == node_get_type(body));
    nusmv_assert(upper_bound >= 0);
  }
  else {
    /* array has to be array var or array define but it is not.
       Actually, since type checking is not done yet LHS of array subscripting
       can be invalid expression.
    */
    error_lhs_of_index_is_not_array();
    /* TODO: we may get the type of expression and obtain the bounds
       from their. Then constraints of having var array or array define
       can be removed */
  }

  /* Build the out of bounds case */
  node_ptr res = failure_make("array access out of bounds",
                              FAILURE_ARRAY_OUT_OF_BOUNDS,
                              yylineno);
  int i;
  SymbType_ptr indextype =
    TypeChecker_get_expression_type(SymbTable_get_type_checker(symb_table),
                                    index, Nil);

  /* Optimization: if the index is a word (which have a limited domain)
     we may limit the range of indexes to word domain */
  if (SymbType_is_word(indextype)) {
    int size = SymbType_get_word_width(indextype);
    /* signed words have 1 bit less for max values */
    if (SymbType_is_signed_word(indextype)) size -= 1;
    /* max possible value which is (2^size - 1) */
    WordNumberValue maxValue = WordNumber_max_unsigned_value(size);

    /* Limit the minimal value:
       unsigned word has minimal value 0
       the singed one has minimal value -(2^(size-1)), i.e. -(max-value+1) */
    if (SymbType_is_unsigned_word(indextype) && lower_bound < 0) {
      lower_bound = 0;
    }
    else if (SymbType_is_signed_word(indextype) &&
             lower_bound < 0 &&
             (-lower_bound) > (maxValue + 1)) {
      lower_bound = -(maxValue + 1);
      nusmv_assert(lower_bound < 0); /* no overflow is possible here */
    }

    /* upper bound cannot be greater than max word value */
    if (upper_bound > 0 /* to avoid signed/unsigned comparisons and casts */ &&
        upper_bound > maxValue) {
      upper_bound = maxValue;
      nusmv_assert(maxValue == upper_bound); /* overflow detection */
    }
  }

  for (i=upper_bound; i>=lower_bound; --i) {
    node_ptr num;
    node_ptr eq;
    node_ptr body;

    if (SymbType_is_word(indextype)) {
      int size = SymbType_get_word_width(indextype);
      if (SymbType_is_signed_word(indextype)) {
        num = find_node(NUMBER_SIGNED_WORD,
                        (node_ptr) WordNumber_from_signed_integer(
                                               (WordNumberValue) i, size),
                        Nil);
      }
      else {
        num = find_node(NUMBER_UNSIGNED_WORD,
                        (node_ptr) WordNumber_from_integer(
                                               (WordNumberValue) i, size),
                        Nil);
      }
    }
    else {
      num = find_node(NUMBER, NODE_FROM_INT(i), Nil);
    }
    eq = Expr_equal(index, num, symb_table);
    body = find_node(ARRAY, array, num);
    /* it is necessary to fully resolve newly created ARRAY
       expression for 2 reasons:
       1) the expression can be a define and the flatting mode
          is set to define-expansion.
       2) the array ID (left child of ARRAY) may be a define
          which are always resolved in ARRAY expression. */
    body = compileFlattenSexpRecur(symb_table, body, Nil);
    if (is_array_next) body = find_node(NEXT, body, Nil);
    res = Expr_ite(eq, body, res, symb_table);
  }
  return res;
}


/**Function********************************************************************

   Synopsis    [Pushes the index-access operator down
   to if-then-else expressions leaves.]

   Description [An index-access operator can be applied
   to if-then-else expression. In such case this function is used
   to push the index-access operator down. E.g.
   (a ? b : c)[i] will be converted to (a ? b[i] : c[i]).

   Flag is_array_next signals that array expression has to
   be wrapped in next whereas index remains intact.

   If idx is a variable index multiplexer is generated (on
   leaves).]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr push_array_index_down(node_ptr array, node_ptr index,
                                       boolean is_array_next,
                                       SymbTable_ptr st)
{
  switch (node_get_type(array)) {
  case CASE:
  case IFTHENELSE: {
    node_ptr left, right;
    left = push_array_index_down(car(array), index, is_array_next, st);
    right = push_array_index_down(cdr(array), index, is_array_next, st);
    return find_node(node_get_type(array), left, right);
  }

  case COLON: {
    node_ptr left = car(array);
    if (is_array_next) left = find_node(NEXT, left, Nil);

    node_ptr right
      = push_array_index_down(cdr(array), index, is_array_next, st);

    return find_node(COLON, left, right);
  }

    /* --- a leave is reached --- */
  case FAILURE:  /* failure remains unchanged */
    return array;

  case NEXT:  /* next is passed as parameter */
    nusmv_assert(!is_array_next); /* double next */
    return push_array_index_down(car(array), index, true, st);

  default:  /* a normal expression */
    {
      node_ptr expr;
      /* it is necessary to put next around array.
         Optimization: if index is a number then next can be put around
         whole array expression. */
      if (NUMBER == node_get_type(index)) /* words were converted to int */ {
        expr = find_node(ARRAY, array, index);
        if (is_array_next) expr = find_node(NEXT, expr, Nil);
      }
      else {
        expr = array;
        if (is_array_next) expr = find_node(NEXT, array, Nil);
        expr = find_node(ARRAY, expr, index);
      }
      /* Here we again invoke compileFlattenSexpRecur.
         There are 2 reasons to do so:

         1) If index is a dynamic expression then created array
            expression has to be resolved by multiplexer (i.e.
            construct_array_multiplexer) but we are lazy to compute
            bound thus we simply invoke
            compileFlattenSexpRecur to do whatever is required

         2) Define expansion may be required to be done.  Again we are
            lazy to check here if a generated expression is a define
            or not. Thus we simply invoke compileFlattenSexpRecur
            to do the job.
      */
      return  compileFlattenSexpRecur(st, expr, Nil);
    }
  } /* switch */
}



/**Function********************************************************************

   Synopsis           [Tries to resolve recursively to a number]

   Description        [This is a private service of function
   CompileFlatten_resolve_number]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr compile_flatten_eval_number(SymbTable_ptr st,
                                            node_ptr n, node_ptr context)
{
  if ((node_ptr) NULL == n) return (node_ptr) NULL;

  switch (node_get_type(n)) {

  case CONTEXT:
    nusmv_assert((node_ptr) NULL == context);
    return compile_flatten_eval_number(st, cdr(n), car(n));

    /* leaves */
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case FAILURE:
    return find_atom(n);

  case ARRAY: {
    ResolveSymbol_ptr rs;
    node_ptr name;

    rs = SymbTable_resolve_symbol(st, n, context);
    name = ResolveSymbol_get_resolved_name(rs);
    /* it might be a symbol that evaluates to a number */

    if (ResolveSymbol_is_undefined(rs)) {
      /* this is array expression. Still it potentially may be
         resolved to constant, e.g. if define v := [1,2,3], define d:=v, then
         d[1] is array expression but can be resolved to 2.
      */
      node_ptr tmp = compileFlattenSexpRecur(st, n, context);
      /* it is impossible that flattening returned undefined identifier. */
      nusmv_assert(tmp != n);
      return compile_flatten_eval_number(st, tmp, Nil);
    }
    else {
      /* array is actually identifier-with-brackets => process it with
         ATOM, DOT and BIT below */
    }
  }
  /* !! NO BREAK HERE !! */


  case ATOM:
  case DOT:
  case BIT: {
    ResolveSymbol_ptr rs;
    /* it might be a symbol that evaluates to a number */
    node_ptr name;

    rs = SymbTable_resolve_symbol(st, n, context);

    name = ResolveSymbol_get_resolved_name(rs);

    if (name != Nil && ResolveSymbol_is_define(rs)) {
      /* retrieves the define value, and checkes if it is a numeric constant */
      node_ptr body = SymbTable_get_define_flatten_body(st, name);
      return compile_flatten_eval_number(st, body, (node_ptr) NULL);
    }
    if (ResolveSymbol_is_parameter(rs)) {
      /* is it a formal parameter? tries with the corresponding actual
         parameter */
      node_ptr actual = SymbTable_get_flatten_actual_parameter(st, name);
      return compile_flatten_eval_number(st, actual, Nil);
    }
    return name;
  }

    /* These nodes need special treatment when used with
       Expr_resolve, since recursively enter into their cdr may
       break the formula. (Ranges with min = max are resolved as
       number by Expr_resolve). See issue 2194. */
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU:
    nusmv_assert(Nil == cdr(n) || TWODOTS == node_get_type(cdr(n)));

    return Expr_resolve(st, node_get_type(n),
                        compile_flatten_eval_number(st, car(n), context),
                        cdr(n));
    break;

  default:
    return Expr_resolve(st, node_get_type(n),
                        compile_flatten_eval_number(st, car(n), context),
                        compile_flatten_eval_number(st, cdr(n), context));
  }
}


/**Function********************************************************************

   Synopsis           [Traverses the list of values, and declare all
   constants (leaves) it finds]

   Description        [Constants will occur within the given layer]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void flatten_declare_constants_within_list(SymbTable_ptr symb_table,
                                                  SymbLayer_ptr layer,
                                                  node_ptr value_list)
{
  while (value_list != Nil) {
    node_ptr name = car(value_list);

    /* HERE we cannot use the ResolveSymbol routines also for simple
       ATOM constants: this happens because being the symbol still NOT
       declared in the symbol table, it will not be recognized as a
       simple ATOM constant, and the get_resolved_name method will
       return a dotted notation node, which leads to problems in
       constant resolution later.. */
    if (node_get_type(name) == DOT) {
      ResolveSymbol_ptr rs;
      rs = SymbTable_resolve_symbol(symb_table, name, Nil);
      name = ResolveSymbol_get_resolved_name(rs);
    }
    else name = find_atom(name);

    if (SymbLayer_can_declare_constant(layer, name) &&
        (!SymbTable_is_symbol_declared(symb_table, name))) {

      SymbLayer_declare_constant(layer, name);
    }
    else {
      if (!SymbTable_is_symbol_constant(symb_table, name)) {
        error_redefining(name);
      }
    }
    value_list = cdr(value_list);
  }
}


/**Function********************************************************************

   Synopsis    [Instantiates the elements of an array define]

   Description [For every cell and every dimension create a correct
   binding in the symbol layer]


   SideEffects [Elements are added to the layer an the symbol table]

   SeeAlso     []

******************************************************************************/
static void
instantiate_array_define(SymbTable_ptr st,
                          SymbLayer_ptr layer,
                          node_ptr name,
                          node_ptr mod_name,
                          node_ptr definition)
{
  if (!SymbLayer_can_declare_define(layer, name)) {
    error_redefining(name);
    error_unreachable_code();
  }

  switch (node_get_type(definition)) {
  case ARRAY_DEF:
    {
      node_ptr iter;
      int idx;

      nusmv_assert((cdr(definition) == Nil) &&
                   "Wrong node arity found: ARRAY_DEF must be unary!");

      /* Declare this symbol */
      SymbLayer_declare_array_define(layer, name, mod_name, definition);

      /* Instantiate every element of the array individually
         with first index = 0 */
      for (idx = 0, iter = car(definition);
           iter != Nil;
           idx += 1, iter = cdr(iter)) {
        /* definition has to be a list of values */
        nusmv_assert(CONS == node_get_type(iter));

        /* Instantiate name[idx] element */
        node_ptr index = find_node(NUMBER, NODE_FROM_INT(idx), Nil);
        instantiate_array_define(st, layer,
                                  find_node(ARRAY, name, index),
                                  mod_name, car(iter));
      }
      break;
    }

  default:
    {
      /* Declare this element */
      SymbLayer_declare_define(layer, name, mod_name, definition);
    }
  }
}


/**Function********************************************************************

  Synopsis           [Add an assign declaration in hrc_result.]

  Description        [Add an assign declaration in hrc_result. The
  type of assign is inferred by the node type found.]

  SideEffects        [Contents of hrc_result is changed adding an
  assign constraint.]

  SeeAlso            []

******************************************************************************/
static void compile_insert_assign_hrc(HrcNode_ptr hrc_result,
                                      node_ptr cur_decl)
{
  if (HRC_NODE(NULL) != hrc_result) {
    node_ptr assign_list = car(cur_decl);

    while (Nil != assign_list) {
      node_ptr assign_body = cdr(assign_list);
      node_ptr left_expr = car(assign_body);
      node_ptr right_expr = cdr(assign_body);

      /* determine init/next/invar part of an assign */
      switch (node_get_type(left_expr)) {
      case SMALLINIT:
        {
          /* init assign */
          node_ptr assign_node = new_node(ASSIGN, car(left_expr), right_expr);
          HrcNode_add_init_assign_expr(hrc_result, assign_node);
        }

        break;
      case NEXT:
        {
          /* next assign */
          node_ptr assign_node = new_node(ASSIGN, car(left_expr), right_expr);
          HrcNode_add_next_assign_expr(hrc_result, assign_node);
        }
        break;
      default:
        {
          /* Invar assign */
          node_ptr assign_node = new_node(ASSIGN, left_expr, right_expr);
          HrcNode_add_invar_assign_expr(hrc_result, assign_node);
        }
      }

      assign_list = car(assign_list);
    } /* end while on assign_list */
  }

  return;
}



/**Function********************************************************************

   Synopsis           [Get the HRC root node from a child]

   Description        [Get the HRC root node from a child]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static HrcNode_ptr get_hrc_root_node (HrcNode_ptr node)
{
  HrcNode_ptr res = node;
  while (!HrcNode_is_root(res)) {
    res = HrcNode_get_parent(res);
  }
  return res;
}


/**Function********************************************************************

   Synopsis           [Given a numeric constant in node_ptr representation
   the function returns its value as int]

   Description        [It is an error if overflow/underflow happens]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int compile_flatten_get_int(node_ptr value)
{
  int res;
  WordNumberValue tmp;
  /* get the constants */
  switch (node_get_type(value)) {
  case NUMBER: res = node_get_int(value); break;
  case NUMBER_UNSIGNED_WORD:
    tmp = WordNumber_get_unsigned_value(WORD_NUMBER(car(value)));
    res = tmp;
    nusmv_assert(res == tmp); /* overflow detection */
    break;
  case NUMBER_SIGNED_WORD:
    tmp = WordNumber_get_unsigned_value(WORD_NUMBER(car(value)));
    res = tmp;
    nusmv_assert(res == tmp); /* overflow detection */
    break;
  default: /* error: value is not a constant */
    error_unreachable_code(); /* only numeric constants can be here */
  }
  return res;
}

/**Function********************************************************************

   Synopsis           [Aux function for the CompileFlatten_normalise_value_list]

   Description         [The normalisation includes: all TRUE and FALSE
   constants are substituted by 1 and 0 numbers]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr compile_flatten_normalise_value_list(node_ptr old_value_list)
{
  node_ptr new_tail;
  node_ptr new_head;

  /* the list is empty */
  if (old_value_list == Nil) return Nil;

  /* normalise the tail */
  new_tail = compile_flatten_normalise_value_list(cdr(old_value_list));

  /* normalise the head */
  new_head = car(old_value_list);

  /* create a new list with the line info kept */
  return new_lined_node(CONS, new_head, new_tail, node_get_lineno(old_value_list));
}

/**Function********************************************************************

   Synopsis           [Aux function for the
                       compile_flatten_rewrite_word_toint_cast, which
                       is used for toint cast operator rewriting]

   Description        [Creates the following expression:
                       wexpr[bit:bit] (is_neg ? "!=" : "=") 0ud1_1 ? (2^bit) : 0

                       For example, for wexpr = "word_var", bit = "2",
                       is_neg = "false" we have:

                       word_var[2:2] = 0ud1_1 ? 4 : 0]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr
compile_flatten_build_word_toint_ith_bit_case(node_ptr wexpr,
                                              int bit,
                                              boolean is_neg)
{
  node_ptr cond, bit_node, mul, zero, w1, res;
  int operator, mul_int, i;

  nusmv_assert(bit <= 32);

  mul_int = 1;
  for (i = 0; i < bit; ++i) { mul_int = mul_int << 1; }

  operator = (is_neg ? NOTEQUAL : EQUAL);

  bit_node = find_node(NUMBER, NODE_FROM_INT(bit), Nil);
  mul = find_node(NUMBER, NODE_FROM_INT(mul_int), Nil);
  zero = find_node(NUMBER, NODE_FROM_INT(0), Nil);
  w1 = find_node(NUMBER_UNSIGNED_WORD,
                 NODE_PTR(WordNumber_from_integer((WordNumberValue)1, 1)),
                 Nil);

  /* w[bit:bit] (= !=) 0ud1_1 */
  cond = new_node(operator,
                  new_node(BIT_SELECTION, wexpr,
                           new_node(COLON, bit_node, bit_node)),
                  w1);

  /* cond ? 2^bit : 0 */
  res = new_node(IFTHENELSE, new_node(COLON, cond, mul), zero);

  return res;
}

/**Function********************************************************************

   Synopsis           [Rewrites the toint operator for word expressions
                       conversion]

   Description        [This functions takes a word expression and rewrites it
                       as a circuit in order to convert the word
                       expression into an integer expression.

                       For unsigned word[N], we rewrite the operator as follows:

                       (w[0:0] = 0ud1_1 ? 1 : 0) +
                       (w[1:1] = 0ud1_1 ? 2 : 0) +
                       ..... +
                       (w[N-1:N-1] = 0ud1_1 ? 2^(N-1) : 0)

                       For signed word[N], we do the following:
                       case
                       w[N-1:N-1] = 0ud1_0 :
                       (w[0:0] = 0ud1_1 ? 1 : 0) +
                       (w[1:1] = 0ud1_1 ? 2 : 0) +
                       ..... +
                       (w[N-2:N-2] = 0ud1_1 ? 2^(N-2) : 0);
                       TRUE:
                       -((w[0:0] = 0ud1_1 ? 0 : 1) +
                       (w[1:1] = 0ud1_1 ? 0 : 2) +
                       ..... +
                       (w[N-2:N-2] = 0ud1_1 ? 0 : 2^(N-2)) + 1);
                       esac]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr
compile_flatten_rewrite_word_toint_cast(node_ptr body, SymbType_ptr type)
{
  node_ptr result = Nil;
  int width = SymbType_get_word_width(type);
  int i;

  nusmv_assert(SymbType_is_word(type));


  if (SymbType_is_unsigned_word(type)) {
    /* Check the width of the word. For unsigned words, the limit is
       31 bits, because NuSMV ints are signed and limited to 32
       bits. */
    if (width > 31) {
      error_out_of_bounds_word_toint_cast(body);
    }

    result =
      compile_flatten_build_word_toint_ith_bit_case(body, 0, false);

    for (i = 1; i < width; ++i) {
      node_ptr tmp =
        compile_flatten_build_word_toint_ith_bit_case(body, i, false);
      result = new_node(PLUS, result, tmp);
    }
  }
  else if (SymbType_is_signed_word(type)) {
    node_ptr w0 = find_node(NUMBER_UNSIGNED_WORD,
                            NODE_PTR(WordNumber_from_integer((WordNumberValue)0, 1)),
                            Nil);
    node_ptr msb = find_node(NUMBER, NODE_FROM_INT(width-1), Nil);
    node_ptr cond, positive, negative;
    int i;

    /* Check the width of the word. For unsigned words, the limit is
       32 bits, because NuSMV ints are signed and limited to 32
       bits. */
    if (width > 32) {
      error_out_of_bounds_word_toint_cast(body);
    }

    /* w[msb:msb] = 0ud1_0 --> IS POSITIVE */
    cond = new_node(EQUAL,
                    new_node(BIT_SELECTION, body,
                             new_node(COLON, msb, msb)),
                    w0);

    /* Prepare the base steps for the sum (LSB) */
    positive =
      compile_flatten_build_word_toint_ith_bit_case(body, 0, false);
    negative =
      compile_flatten_build_word_toint_ith_bit_case(body, 0, true);

    /* Sum all other bits (exept the MSB) */
    for (i = 1; i < (width - 1); ++i) {
      node_ptr tmp =
        compile_flatten_build_word_toint_ith_bit_case(body, i, false);
      positive = new_node(PLUS, positive, tmp);

      tmp = compile_flatten_build_word_toint_ith_bit_case(body, i, true);
      negative = new_node(PLUS, negative, tmp);
    }

    negative = new_node(PLUS, negative,
                        find_node(NUMBER, NODE_FROM_INT(1), Nil));
    negative = new_node(UMINUS, negative, Nil);

    /* positive_word ? positive_circuit : negative_circuit */
    result = new_node(CASE, new_node(COLON, cond, positive), negative);
  } /* Is signed word */

  return result;
}
