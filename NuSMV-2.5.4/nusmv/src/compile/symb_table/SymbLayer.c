/**CFile*****************************************************************

  FileName    [SymbLayer.c]

  PackageName [compile.symb_table]

  Synopsis    [Implementation of the system-wide SymbLayer]

  Description []

  SeeAlso     [SymbLayer.h]

  Author      [Roberto Cavada, Alessandro Mariotti]

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

#include "SymbLayer.h"
#include "SymbLayer_private.h"
#include "SymbCache.h"
#include "SymbCache_private.h"
#include "symb_table_int.h"

#include "compile/compileInt.h"
#include "compile/symb_table/NFunction.h"
#include "parser/symbols.h"

#include "utils/error.h"
#include "utils/NodeList.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Type***********************************************************************

  Synopsis          [A SymbLayer is a colleciont of different kind of symbols,
  like variables, DEFINES, constants, etc. After it had been filled
  with symbols, it can must be committed to a encoding in order to
  have the symbols it contains converted to a form that depends on the
  specific encoding type (for example BDDs, Bes, booleanized scalar
  variables). ]

  Description        [A layer always belongs to a
  SymbTable instance, that can hold many layers in a stack, and that
  is responsible for their life cycles.
  A given layer is identified within the symbol table it belongs to by
  a name as a string of characters.

  The layer must be used as a container of symbols, to declare new
  symbols within an encoding. First a layer is filled in with symbols,
  then it is committed to one or more encodings like BoolEnc, BddEnc
  or BeEnc.

  When a layer is no longer useful, it must be removed from all the
  encodings it had been committed to, and then can be removed from
  the symbol table as well. ]

******************************************************************************/
typedef struct SymbLayer_TAG {
  char* name;
  LayerInsertPolicy insert_policy;

  /* the current number of encodings self is registered with */
  int committed_to_encs;

  /* Insertion ordered list */
  node_ptr* symbols;
  unsigned int symbols_allocated;
  unsigned int symbols_index;
  unsigned int symbols_empty;

  /* For removal in constant time */
  hash_ptr symbol2position;

  SymbCache_ptr cache;

  /* Counters */
  int constants_num;
  int state_vars_num;
  int input_vars_num;
  int frozen_vars_num;
  int defines_num;
  int functions_num;
  int array_defines_num;
  int variable_arrays_num;
  int parameters_num;

  int bool_state_vars_num;
  int bool_input_vars_num;
  int bool_frozen_vars_num;
} SymbLayer;



/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define INITIAL_SYMBOLS_ARRAY_SIZE 8

#define IS_SYMBOL_UNDEF(self, sym)                      \
  (Nil == find_assoc(self->symbol2position, sym))

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void symb_layer_init ARGS((SymbLayer_ptr self, const char* name,
                                  const LayerInsertPolicy policy,
                                  SymbCache_ptr cache));

static void symb_layer_deinit ARGS((SymbLayer_ptr self, boolean clean_cache));

static inline void
symb_layer_check_and_shrink_symbols ARGS((SymbLayer_ptr self));


static inline void
symb_layer_remove_symbol ARGS((SymbLayer_ptr self, const node_ptr sym));

static inline void
symb_layer_new_symbol ARGS((SymbLayer_ptr self, const node_ptr sym));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Class SymbLayer constructor]

  Description        [name is copied, the caller keeps ownership of cache.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbLayer_ptr SymbLayer_create(const char* name,
                               const LayerInsertPolicy policy,
                               SymbCache_ptr cache)
{
  SymbLayer_ptr self = ALLOC(SymbLayer, 1);

  SYMB_LAYER_CHECK_INSTANCE(self);

  symb_layer_init(self, name, policy, cache);
  return self;
}


/**Function********************************************************************

  Synopsis           [Class SymbLayer destructor]

  Description        [Use this destructor if the SymbCache will not be
                      destroyed after this call (ie. You are removing
                      a layer from the Symbol Table)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_destroy(SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  symb_layer_deinit(self, true);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [Class SymbLayer destructor]

  Description        [Use this destructor if the SymbCache will be
                      destroyed after this call (ie. You are
                      destroying the Symbol Table)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_destroy_raw(SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  symb_layer_deinit(self, false);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Sets the layer name.]

  Description        [This method is protected (not usable by users, only
  used by the symbol table when renaming a layer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_set_name(SymbLayer_ptr self, const char* new_name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  /* frees the previous name if needed */
  if (self->name != (char*) NULL) { FREE(self->name); }
  if (new_name != (char*) NULL)  self->name = util_strsav((char*) new_name);
  else self->name = (char*) NULL;
}


/**Function********************************************************************

  Synopsis           [Returns the name self had been registered with.]

  Description        [Returned string must not be freed, it belongs to self]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
const char* SymbLayer_get_name(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return (const char*) self->name;
}


/**Function********************************************************************

  Synopsis           [Returns the policy that must be adopted to stack this
  layer into a layers stack, within a SymbTable instance]

  Description        [This method is thought to be used exclusively by class
  SymbTable]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
LayerInsertPolicy SymbLayer_get_insert_policy(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  if (self->insert_policy == SYMB_LAYER_POS_DEFAULT) return SYMB_LAYER_POS_BOTTOM;
  else return self->insert_policy;
}


/**Function********************************************************************

  Synopsis          [Compares the insertion policies of self and other, and
 returns true if self must be inserted *before* other]

  Description        [Compares the insertion policies of self and other, and
  returns true if self must be inserted *before* other.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_must_insert_before(const SymbLayer_ptr self,
                                     const SymbLayer_ptr other)
{
  LayerInsertPolicy p1, p2;

  SYMB_LAYER_CHECK_INSTANCE(self);

  p1 = SymbLayer_get_insert_policy(self);
  p2 = SymbLayer_get_insert_policy(other);

  /* checks that p1 and p2 do not refer the same forced position */
  nusmv_assert((p1 != p2) || ((p1 != SYMB_LAYER_POS_FORCE_TOP) &&
                              (p1 != SYMB_LAYER_POS_FORCE_BOTTOM)));

  switch (p1) {
  case SYMB_LAYER_POS_FORCE_TOP: return true;

  case SYMB_LAYER_POS_TOP:
    return p2 != SYMB_LAYER_POS_FORCE_TOP;

  case SYMB_LAYER_POS_DEFAULT:
  case SYMB_LAYER_POS_BOTTOM: return p2 == SYMB_LAYER_POS_FORCE_BOTTOM;

  default:
    internal_error("Unexpected layer insertion policy");
  }

  return false;
}



/**Function********************************************************************

  Synopsis          [Called every time an instance is committed within an
  encoding.]

  Description        [This method is part of a private registration protocol
  between encodings and layers, and must be considered as a private
  method.  Every time a layer is registered (committed) within an
  enconding, the layer is notified with a call to this method from the
  encoding instance which the layer is committed to. This mechanism
  helps to detect errors when a layer in use by an encoding is removed
  and destroyed from within a symbol table. The destructor will always
  check that self is not in use by any encoding when it is invoked.]

  SideEffects        []

  SeeAlso            [removed_from_enc]

******************************************************************************/
void SymbLayer_committed_to_enc(SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  self->committed_to_encs += 1;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stdout,
            "SymbLayer '%s': committed to %d encodings\n",
            SymbLayer_get_name(self), self->committed_to_encs);
  }
}


/**Function********************************************************************

  Synopsis          [Called every time an instance is removed from an
  encoding.]

  Description       [This method is part of a private registration protocol
  between encodings and layers, and must be considered as a private
  method.  Every time a layer is removed (uncommitted) from an
  enconding, the layer is notified with a call to this method from the
  encoding instance which the layer is being removed from. This mechanism
  helps to detect errors when a layer in use by an encoding is removed
  and destroyed from within a symbol table. The destructor will always
  check that self is not in use by any encoding when it is invoked.]

  SideEffects        []

  SeeAlso            [commit_to_enc]

******************************************************************************/
void SymbLayer_removed_from_enc(SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  nusmv_assert(self->committed_to_encs > 0);

  self->committed_to_encs -= 1;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stdout,
            "SymbLayer '%s': removed from encoding (%d remaining)\n",
            SymbLayer_get_name(self), self->committed_to_encs);
  }
}

/**Function********************************************************************

  Synopsis           [Call this method to know if a new NFunction can be
  declared within this layer.]

  Description        [Returns true if the given symbol does not exist
  within the symbol table which self belongs to. Returns
  false if the symbol was already declared. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_can_declare_function(const SymbLayer_ptr self,
                                       const node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return !SymbCache_is_symbol_declared(self->cache, name);
}



/**Function********************************************************************

  Synopsis           [Call this method to know if a new constant can be
  declared within this layer.]

  Description        [Since more than one layer can declare the same constants,
  this method might return true even if another layer already contain the
  given constant. If the constant had already been declared within self,
  false is returned. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_can_declare_constant(const SymbLayer_ptr self,
                                       const node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return IS_SYMBOL_UNDEF(self, name);
}


/**Function********************************************************************

  Synopsis           [Call this method to know if a new variable can be
  declared within this layer.]

  Description        [Returns true if the given symbol does not exist
  within the symbol table which self belongs to. Returns
  false if the symbol was already declared. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_can_declare_var(const SymbLayer_ptr self,
                                  const node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return !SymbCache_is_symbol_declared(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Call this method to know if a new DEFINE can be
  declared within this layer.]

  Description        [Returns true if the given symbol does not exist within
  the symbol table which self belongs to. Returns false if the symbol
  was already declared. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_can_declare_define(const SymbLayer_ptr self,
                                     const node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return !SymbCache_is_symbol_declared(self->cache, name);
}

/**Function********************************************************************

  Synopsis           [Call this method to know if a new parameter can be
                      declared within this layer.]

  Description        [Returns true if the given symbol does not exist within
  the symbol table which self belongs to. Returns false if the symbol
  was already declared. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_can_declare_parameter(const SymbLayer_ptr self,
                                        const node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return !SymbCache_is_symbol_declared(self->cache, name);
}

/**Function********************************************************************

  Synopsis           [Call this method to know if a new define array can be
  declared within this layer.]

  Description        [Returns true if the given symbol does not exist within
  the symbol table which self belongs to. Returns false if the symbol
  was already declared. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_can_declare_array_define(const SymbLayer_ptr self,
                                     const node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return !SymbCache_is_symbol_declared(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Call this method to know if a new variable_array can be
  declared within this layer.]

  Description        [Returns true if the given symbol does not exist within
  the symbol table which self belongs to. Returns false if the symbol
  was already declared. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_can_declare_variable_array(const SymbLayer_ptr self,
                                          const node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return !SymbCache_is_symbol_declared(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Insert a new NFunction]

  Description        [A new NFunction is declared within the layer.
                      Name must be contestualized, context is provided
                      as a separated information]

  SideEffects        []

  SeeAlso            [SymbLayer_can_declare_function]

******************************************************************************/
void SymbLayer_declare_function(SymbLayer_ptr self, node_ptr name,
                                  node_ptr ctx, NFunction_ptr fun)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  nusmv_assert(SymbLayer_can_declare_function(self, name));

  SymbCache_new_function(self->cache, name, ctx, fun);

  symb_layer_new_symbol(self, name);

  ++self->functions_num;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new function '",
            self->name);
    print_node(nusmv_stdout, name);
    fprintf(nusmv_stdout, "'\n");
  }
}

/**Function********************************************************************

  Synopsis           [Insert a new constant]

  Description        [A new constant is created]

  SideEffects        []

  SeeAlso            [SymbLayer_can_declare_constant]

******************************************************************************/
void SymbLayer_declare_constant(SymbLayer_ptr self, node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  nusmv_assert(SymbLayer_can_declare_constant(self, name));

  SymbCache_new_constant(self->cache, name);

  symb_layer_new_symbol(self, name);

  ++self->constants_num;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new constant '",
            self->name);
    print_node(nusmv_stdout, name);
    fprintf(nusmv_stdout, "'\n");
  }
}


/**Function********************************************************************

  Synopsis           [Insert a new input variable]

  Description        [A new input variable is created of a given type.
  The variable type can be created with SymbType_create or returned by
  funtions SymbTablePkg_..._type.
  The layer is responsible for destroying the variable's type.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_declare_input_var(SymbLayer_ptr self, node_ptr var_name,
                                 SymbType_ptr type)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  /* not already declared */
  if (!SymbLayer_can_declare_var(self, var_name)) {
    fprintf(stderr, "Error: Cannot declare input variable ");
    print_node(stderr, var_name);
    internal_error("Symbol already declared");
  }

  SymbCache_new_input_var(self->cache, var_name, type);

  symb_layer_new_symbol(self, var_name);

  ++self->input_vars_num;
  if (SymbType_is_boolean(type)) {
    ++self->bool_input_vars_num;
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new input variable '",
            self->name);
    print_node(nusmv_stdout, var_name);
    fprintf(nusmv_stdout, "'\n");
  }

}


/**Function********************************************************************

  Synopsis           [Insert a new state variable]

  Description        [A new state variable is created of a given type.
  The variable type can be created with SymbType_create or returned by
  funtions SymbTablePkg_..._type.
  The layer is responsible for destroying the variable's type.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_declare_state_var(SymbLayer_ptr self, node_ptr var_name,
                                 SymbType_ptr type)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  if (! SymbLayer_can_declare_var(self, var_name)) {
    fprintf(stderr, "Error: cannot declare state variable ");
    print_node(stderr, var_name);
    internal_error("Symbol already declared");
  }

  SymbCache_new_state_var(self->cache, var_name, type);

  symb_layer_new_symbol(self, var_name);

  ++self->state_vars_num;
  if (SymbType_is_boolean(type)) {
    ++self->bool_state_vars_num;
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new state variable '",
            self->name);
    print_node(nusmv_stdout, var_name);
    fprintf(nusmv_stdout, "'\n");
  }
}


/**Function********************************************************************

  Synopsis           [Insert a new frozen variable]

  Description        [A new frozen variable is created of a given type.
  The variable type can be created with SymbType_create or returned by
  funtions SymbTablePkg_..._type.
  The layer is responsible for destroying the variable's type.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_declare_frozen_var(SymbLayer_ptr self, node_ptr var_name,
                                  SymbType_ptr type)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  if (! SymbLayer_can_declare_var(self, var_name)) {
    fprintf(stderr, "Error: cannot declare frozen variable ");
    print_node(stderr, var_name);
    internal_error("Symbol already declared");
  }

  SymbCache_new_frozen_var(self->cache, var_name, type);

  symb_layer_new_symbol(self, var_name);

  ++self->frozen_vars_num;
  if (SymbType_is_boolean(type)) {
    ++self->bool_frozen_vars_num;
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new frozen variable '",
            self->name);
    print_node(nusmv_stdout, var_name);
    fprintf(nusmv_stdout, "'\n");
  }
}


/**Function********************************************************************

  Synopsis           [Redeclare a state variable as a frozen variable]

  Description        [A variable is frozen if it is known then the var's value
  cannot change in transitions.
  'var' must be a state variable already defined and not redeclared as frozen.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
EXTERN void
SymbLayer_redeclare_state_as_frozen_var(SymbLayer_ptr self, node_ptr var)
{
  SymbType_ptr type;
  SymbCache_redeclare_state_as_frozen_var(self->cache, var);

  ++self->frozen_vars_num;
  --self->state_vars_num;

  type = SymbCache_get_var_type(self->cache, var);

  if (SymbType_is_boolean(type)) {
    ++self->bool_frozen_vars_num;
    --self->bool_state_vars_num;
  }

}


/**Function********************************************************************

  Synopsis           [Insert a new DEFINE]

  Description        [A new DEFINE of a given value is created. name must be
  contestualized, context is provided as a separated information]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_declare_define(SymbLayer_ptr self, node_ptr name,
                              node_ptr context, node_ptr definition)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  if (!SymbLayer_can_declare_define(self, name)) {
    fprintf(stderr, "Error: cannot declare DEFINE ");
    print_node(stderr, name);
    internal_error("SymbLayer_declare_define: name already declared\n");
  }

  SymbCache_new_define(self->cache, name, context, definition);

  symb_layer_new_symbol(self, name);

  ++self->defines_num;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new DEFINE '",
            self->name);
    print_node(nusmv_stdout, name);
    fprintf(nusmv_stdout, "'\n");
  }
}

/**Function********************************************************************

  Synopsis           [Insert a new formal parameters]

  Description        [A new parameter of a given value is created. name must be
  contestualized, context is provided as a separated information]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_declare_parameter(SymbLayer_ptr self, node_ptr formal,
                                 node_ptr context, node_ptr actual)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  if (!SymbLayer_can_declare_parameter(self, formal)) {
    fprintf(stderr, "Error: cannot declare parameter ");
    print_node(stderr, formal);
    internal_error("SymbLayer_declare_parameter: formal param already declared\n");
  }

  SymbCache_new_parameter(self->cache, formal, context, actual);

  symb_layer_new_symbol(self, formal);

  ++self->parameters_num;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new parameter '",
            self->name);
    print_node(nusmv_stdout, formal);
    fprintf(nusmv_stdout, "'\n");
  }
}


/**Function********************************************************************

  Synopsis           [Insert a new array define array]

  Description        [A new define array of a given value is created. name must be
  contestualized, context is provided as a separated information]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_declare_array_define(SymbLayer_ptr self, node_ptr name,
                                    node_ptr context, node_ptr definition)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  if (!SymbLayer_can_declare_array_define(self, name)) {
    fprintf(stderr, "Error: cannot declare define array ");
    print_node(stderr, name);
    internal_error("SymbLayer_declare_define: name already declared\n");
  }

  SymbCache_new_array_define(self->cache, name, context, definition);

  symb_layer_new_symbol(self, name);

  ++self->array_defines_num;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new define array '",
            self->name);
    print_node(nusmv_stdout, name);
    fprintf(nusmv_stdout, "'\n");
  }
}


/**Function********************************************************************

  Synopsis    [Insert a new symbol-type association, i.e. array var ]

  Description [The specified name will be associated to the give array type
  in the symbols collection]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_declare_variable_array(SymbLayer_ptr self, node_ptr name,
                                      SymbType_ptr type)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  if (! SymbLayer_can_declare_variable_array(self, name)) {
    fprintf(stderr, "Error: cannot declare variable_array ");
    print_node(stderr, name);
    internal_error("Symbol already declared");
  }

  SymbCache_new_variable_array(self->cache, name, type);

  symb_layer_new_symbol(self, name);

  ++self->variable_arrays_num;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': declared new variable_array '",
            self->name);
    print_node(nusmv_stdout, name);
    fprintf(nusmv_stdout, "'\n");
  }
}


/**Function********************************************************************

  Synopsis           [Removes a variable previously delcared]

  Description [This method can be called only if self is not
  currently commited to any encoding. It is not allowed to remove
  symbols from layers that are committed to any encoder. This is
  required as caches and other mechanisms may fail to work
  correctly otherwise.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_remove_var(SymbLayer_ptr self, node_ptr name)
{
  SymbType_ptr type;

  /* IMPORTANT: do not remove this assertion! (read comment) */
  nusmv_assert(self->committed_to_encs == 0); /* not in use */

  nusmv_assert(SymbCache_is_symbol_var(self->cache, name));
  
  type = SymbCache_get_var_type(self->cache, name);


  if (SymbCache_is_symbol_state_var(self->cache, name)) {
    --self->state_vars_num;
    if (SymbType_is_boolean(type)) {
      --self->bool_state_vars_num;
    }
  }
  else if (SymbCache_is_symbol_frozen_var(self->cache, name)) {
    --self->frozen_vars_num;
    if (SymbType_is_boolean(type)) {
      --self->bool_frozen_vars_num;
    }
  }
  else if (SymbCache_is_symbol_input_var(self->cache, name)) {
    --self->input_vars_num;
    if (SymbType_is_boolean(type)) {
      --self->bool_input_vars_num;
    }
  }
  else {
    error_unreachable_code();
  }

  /* removes the variable  */
  SymbCache_remove_var(self->cache, name);

  symb_layer_remove_symbol(self, name);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': removed variable '", self->name);
    print_node(nusmv_stdout, name);
    fprintf(nusmv_stdout, "'\n");
  }
}


/**Function********************************************************************

  Synopsis           [Removes a previously declared DEFINE]

  Description [This method can be called only if self is not
  currently commited to any encoding. It is not allowed to remove
  symbols from layers that are committed to any encoder. This is
  required as caches and other mechanisms may fail to work
  correctly otherwise.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_remove_define(SymbLayer_ptr self, node_ptr name)
{
  /* IMPORTANT: do not remove this assertion! (read comment) */
  nusmv_assert(self->committed_to_encs == 0); /* not in use */

  nusmv_assert(SymbCache_is_symbol_define(self->cache, name));

  /* removes the variable  */
  SymbCache_remove_define(self->cache, name);

  symb_layer_remove_symbol(self, name);

  --self->defines_num;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': removed define '", self->name);
    print_node(nusmv_stdout, name);
    fprintf(nusmv_stdout, "'\n");
  }
}

/**Function********************************************************************

  Synopsis           [Removes a previously declared NFunction]

  Description [This method can be called only if self is not
  currently commited to any encoding. It is not allowed to remove
  symbols from layers that are committed to any encoder. This is
  required as caches and other mechanisms may fail to work
  correctly otherwise.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_remove_function(SymbLayer_ptr self, node_ptr name)
{
  /* IMPORTANT: do not remove this assertion! (read comment) */
  nusmv_assert(self->committed_to_encs == 0); /* not in use */

  nusmv_assert(SymbCache_is_symbol_function(self->cache, name));

  /* removes the variable  */
  SymbCache_remove_function(self->cache, name);

  symb_layer_remove_symbol(self, name);

  --self->functions_num;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbLayer '%s': removed function '", self->name);
    print_node(nusmv_stdout, name);
    fprintf(nusmv_stdout, "'\n");
  }
}

/**Function********************************************************************

  Synopsis           [Returns the number of declared symbols]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_symbols_num(const SymbLayer_ptr self)
{
  int res = 0;

  res += self->constants_num;
  res += self->state_vars_num;
  res += self->input_vars_num;
  res += self->frozen_vars_num;
  res += self->defines_num;
  res += self->functions_num;
  res += self->array_defines_num;
  res += self->variable_arrays_num;
  res += self->parameters_num;

  nusmv_assert(res == (self->symbols_index - self->symbols_empty));

  return res;
}

/**Function********************************************************************

  Synopsis           [Returns the number of declared variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_vars_num(const SymbLayer_ptr self)
{
  int res = 0;

  res += self->state_vars_num;
  res += self->input_vars_num;
  res += self->frozen_vars_num;

  return res;
}

/**Function********************************************************************

  Synopsis           [Returns the number of declared contants]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_constants_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->constants_num;
}

/**Function********************************************************************

  Synopsis           [Returns the number of declared state variables.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_state_vars_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->state_vars_num;
}


/**Function********************************************************************

  Synopsis           [Returns the number of declared boolean state variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_bool_state_vars_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->bool_state_vars_num;
}


/**Function********************************************************************

  Synopsis           [Returns the number of declared frozen variables.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_frozen_vars_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->frozen_vars_num;
}


/**Function********************************************************************

  Synopsis           [Returns the number of declared boolean frozen variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_bool_frozen_vars_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->bool_frozen_vars_num;
}


/**Function********************************************************************

  Synopsis           [Returns the number of declared input variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_input_vars_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->input_vars_num;
}


/**Function********************************************************************

  Synopsis           [Returns the number of declared boolean input variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_bool_input_vars_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->bool_input_vars_num;
}


/**Function********************************************************************

  Synopsis           [Returns the number of DEFINEs.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_defines_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->defines_num;
}

/**Function********************************************************************

  Synopsis           [Returns the number of NFunctions.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_functions_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->functions_num;
}

/**Function********************************************************************

  Synopsis           [Returns the number of parameters.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_parameters_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->parameters_num;
}


/**Function********************************************************************

  Synopsis           [Returns the number of define arrays.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_array_defines_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->array_defines_num;
}


/**Function********************************************************************

  Synopsis           [Returns the number of Symbol Types.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbLayer_get_variable_arrays_num(const SymbLayer_ptr self)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return self->variable_arrays_num;
}

/**Function********************************************************************

  Synopsis           [Returns true if the variable is defined in the layer.]

  Description        [Returns true if the variable is defined in the layer.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_is_variable_in_layer(SymbLayer_ptr self, node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);

  /* Must be a symbol in this layer and a variable in the cache */
  return (Nil != find_assoc(self->symbol2position, name) &&
          SymbCache_is_symbol_var(self->cache, name));
}

/**Function********************************************************************

  Synopsis           [Returns true if the symbol is defined in the layer.]

  Description        [Returns true if the symbol is defined in the layer.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_is_symbol_in_layer(SymbLayer_ptr self, node_ptr name)
{
  SYMB_LAYER_CHECK_INSTANCE(self);
  return (Nil != find_assoc(self->symbol2position, name));
}


/**Function********************************************************************

  Synopsis           [Moves the iterator over the next symbol]

  Description        [Moves the iterator over the next symbol,
                      regarding to the mask given when built using
                      SymbCache_gen_iter]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_iter_next(const SymbLayer_ptr self,
                         SymbLayerIter* iter)
{
  node_ptr sym;
  SymbTableType type;
  boolean valid = true;

  /* Iterator not at it's end */
  nusmv_assert(iter->index != self->symbols_index);

  do {
    valid = true;
    ++iter->index;

    /* The end.. */
    if (iter->index == self->symbols_index) {
      break;
    }

    sym = self->symbols[iter->index];

    /* Empty cell, continue with next */
    if (Nil == sym) {
      valid = false;
      continue;
    }

    /* Filter is not valid for this symbol */
    if (((SymbLayerIterFilterFun)NULL != iter->filter) &&
        !(iter->filter(self, sym, iter->arg))) {
      valid = false;
      continue;
    }

    type = SymbCache_get_symbol_type(self->cache, sym);

  } while (!valid || ((type & iter->mask) == 0)); /* Not a match */

}


/**Function********************************************************************

  Synopsis           [Generates an interator over the Symbol Cache symbols]

  Description        [Generates an interator over the Symbol Cache symbols.
                      The iterator will ignore all symbols that do not
                      satisfy the mask]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_gen_iter(const SymbLayer_ptr self,
                        SymbLayerIter* iter,
                        const unsigned int mask)
{
  SymbTableType type;
  node_ptr sym;
  unsigned int index = 0;

  iter->mask = mask;
  iter->filter = (SymbLayerIterFilterFun)NULL;
  iter->arg = NULL;

  /* The list is not empty */
  if (index != self->symbols_index) {
    sym = self->symbols[index];

    type = (Nil != sym) ? \
      SymbCache_get_symbol_type(self->cache, sym) : STT_NONE;

    /* Current symbol does not match */
    while ((Nil == sym) || (type & mask) == 0) {
      ++index;

      /* The end.. */
      if (index == self->symbols_index) { break; }

      sym = self->symbols[index];

      /* Empty cell, continue with next */
      if (Nil == sym) { continue; }

      type = SymbCache_get_symbol_type(self->cache, sym);
    }
  }

  iter->index = index;
}

/**Function********************************************************************

  Synopsis           [Sets the filter for an interator over the
                      Symbol Layer symbols]

  Description        [Sets the filter for an interator over the
                      Symbol Layer symbols. The iterator will be moved
                      in order to point to a symbol that satisfies
                      both the mask and the filter]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbLayer_iter_set_filter(const SymbLayer_ptr self,
                               SymbLayerIter* iter,
                               SymbLayerIterFilterFun filter,
                               void* arg)
{
  unsigned int index = iter->index;
  SymbTableType mask = iter->mask;
  SymbTableType type;
  node_ptr sym;

  iter->filter = filter;
  iter->arg = arg;

  /* The list is not empty */
  if (index != self->symbols_index) {
    sym = self->symbols[index];
    type = SymbCache_get_symbol_type(self->cache, sym);

    /* Current symbol does not match */
    while ((type & mask) == 0 || !(filter(self, sym, arg))) {
      ++index;

      /* The end.. */
      if (index == self->symbols_index) { break; }

      sym = self->symbols[index];

      /* Empty cell, continue with next */
      if (Nil == sym) { continue; }

      type = SymbCache_get_symbol_type(self->cache, sym);
    }
  }

  iter->index = index;
}

/**Function********************************************************************

  Synopsis           [Checks if the iterator is at it's end]

  Description        [Checks if the iterator is at it's end]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_iter_is_end(const SymbLayer_ptr self,
                              const SymbLayerIter* iter)
{
  return self->symbols_index == iter->index;
}


/**Function********************************************************************

  Synopsis           [Get the symbol pointed by the iterator]

  Description        [Get the symbol pointed by the iterator]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr SymbLayer_iter_get_symbol(const SymbLayer_ptr self,
                                   const SymbLayerIter* iter)
{
  nusmv_assert(!SymbLayer_iter_is_end(self, iter));
  return self->symbols[iter->index];
}

/**Function********************************************************************

  Synopsis           [Generates a set starting from the given iterator.]

  Description        [Generates a set starting from the given iterator.
                      The iter will not be consumed (since passed as
                      copy)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t SymbLayer_iter_to_set(const SymbLayer_ptr self, SymbLayerIter iter)
{
  Set_t res = Set_MakeEmpty();

  while (!SymbLayer_iter_is_end(self, &iter)) {
    res = Set_AddMember(res, SymbLayer_iter_get_symbol(self, &iter));
    SymbLayer_iter_next(self, &iter);
  }

  return res;
}

/**Function********************************************************************

  Synopsis           [Generates a list starting from the given iterator.]

  Description        [Generates a list starting from the given iterator.
                      The iter will not be consumed (since passed as
                      copy)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr SymbLayer_iter_to_list(const SymbLayer_ptr self, SymbLayerIter iter)
{
  NodeList_ptr res = NodeList_create();

  while (!SymbLayer_iter_is_end(self, &iter)) {
    NodeList_append(res, SymbLayer_iter_get_symbol(self, &iter));
    SymbLayer_iter_next(self, &iter);
  }

  return res;
}

/**Function********************************************************************

  Synopsis           [Counts the elements of the iterator.]

  Description        [Counts the elements of the iterator.
                      The iter will not be consumed (since passed as
                      copy)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
unsigned int SymbLayer_iter_count(const SymbLayer_ptr self, SymbLayerIter iter)
{
  unsigned int res = 0;

  while (!SymbLayer_iter_is_end(self, &iter)) {
    ++res;
    SymbLayer_iter_next(self, &iter);
  }

  return res;
}

/**Function********************************************************************

  Synopsis           [Boolean Variables filter]

  Description        [SymbLayer built-in filter: Returns true iff the symbol
                      is a boolean variable]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbLayer_iter_filter_bool_vars(const SymbLayer_ptr self,
                                        const node_ptr sym,
                                        void* arg)
{
  return SymbCache_is_symbol_var(self->cache, sym) &&
    SymbType_is_boolean(SymbCache_get_var_type(self->cache, sym));
}

/*--------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private method called by the constructor]

  Description        [Called by the constructor]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
symb_layer_init(SymbLayer_ptr self, const char* name,
                const LayerInsertPolicy policy, SymbCache_ptr cache)
{
  self->name = (char*) NULL;
  SymbLayer_set_name(self, name);

  self->insert_policy = policy;
  self->committed_to_encs = 0;
  self->cache = cache;

  self->symbols_allocated = INITIAL_SYMBOLS_ARRAY_SIZE;
  self->symbols_index = 0;
  self->symbols_empty = 0;
  self->symbols = ALLOC(node_ptr, self->symbols_allocated);

  self->symbol2position = new_assoc();

  /* Counters */
  self->constants_num = 0;
  self->state_vars_num = 0;
  self->input_vars_num = 0;
  self->frozen_vars_num = 0;
  self->bool_state_vars_num = 0;
  self->bool_input_vars_num = 0;
  self->bool_frozen_vars_num = 0;
  self->defines_num = 0;
  self->functions_num = 0;
  self->array_defines_num = 0;
  self->variable_arrays_num = 0;
  self->parameters_num = 0;
}


/**Function********************************************************************

  Synopsis           [Private method called by the destructor]

  Description        [Called by the destructor]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void symb_layer_deinit(SymbLayer_ptr self, boolean clean_cache)
{
  nusmv_assert(self->committed_to_encs == 0); /* not in use by encs */

  /* frees the name */
  if (self->name != (char*) NULL) { FREE(self->name); }

  if (clean_cache) {
    SymbCache_remove_symbols(self->cache, self->symbols, self->symbols_index);
  }

  free_assoc(self->symbol2position);
  FREE(self->symbols);
}

/**Function********************************************************************

  Synopsis           [Adds the given symbol from the layer]

  Description        [Adds the given symbol from the layer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static inline void
symb_layer_new_symbol(SymbLayer_ptr self, const node_ptr sym)
{
  unsigned int index = self->symbols_index;

  /* Index is stored incremented by one, so it is possible to check
     for NULL */
  insert_assoc(self->symbol2position, sym, NODE_FROM_INT(index + 1));

  if (index == self->symbols_allocated) {
    self->symbols_allocated *= 2;
    self->symbols = REALLOC(node_ptr, self->symbols, self->symbols_allocated);
  }

  self->symbols[index] = sym;

  self->symbols_index++;
}

/**Function********************************************************************

  Synopsis           [Removes the given symbol from the layer]

  Description        [Removes the given symbol from the layer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static inline void
symb_layer_remove_symbol(SymbLayer_ptr self, const node_ptr sym)
{
  unsigned int pos = NODE_TO_INT(remove_assoc(self->symbol2position, sym));

  nusmv_assert(0 != pos);

  /* Remove from the list */
  self->symbols[pos - 1] = Nil;
  self->symbols_empty++;

  symb_layer_check_and_shrink_symbols(self);
}

/**Function********************************************************************

  Synopsis           [Shrinks the symbols array if needed]

  Description        [Shrinks the symbols array if needed]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static inline void
symb_layer_check_and_shrink_symbols(SymbLayer_ptr self)
{
  /* ~75% of the list is empty.. shrink */
  if ((self->symbols_allocated > INITIAL_SYMBOLS_ARRAY_SIZE) &&
      ((double)self->symbols_empty / (double)self->symbols_allocated) > 0.75) {
    unsigned int i, j;
    node_ptr* old_symbols = self->symbols;
    node_ptr* new_symbols;

    self->symbols_allocated /= 2;
    new_symbols = ALLOC(node_ptr, self->symbols_allocated);
    self->symbols = new_symbols;

    for (i = 0, j = 0; j < self->symbols_index; ++j) {
      node_ptr sym = old_symbols[j];

      /* The symbol has not been removed.. */
      if (sym != Nil) {
        new_symbols[i] = sym;

        nusmv_assert(i < self->symbols_allocated);

        /* Update the position. Index is stored incremented by one, so
           it is possible to check for NULL */
        insert_assoc(self->symbol2position, sym, NODE_FROM_INT(i + 1));

        ++i;
      }
    }

    self->symbols_index = i;

    /* After shrinking, there are no empty cells */
    self->symbols_empty = 0;

    FREE(old_symbols);
  }
}

