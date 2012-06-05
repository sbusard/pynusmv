/**CFile*****************************************************************

  FileName    [SymbTable.c]

  PackageName [compile.symb_table]

  Synopsis    [Implementation of the system-wide SymbolTable]

  Description []

  SeeAlso     [SymbTable.h]

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

#include "SymbTable.h"

#include "SymbLayer.h"
#include "SymbLayer_private.h"
#include "SymbCache.h"
#include "SymbCache_private.h"
#include "ResolveSymbol.h"


#include "compile/compileInt.h"
#include "compile/type_checking/TypeChecker.h"

#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/NodeList.h"
#include "utils/ustring.h"
#include "utils/assoc.h"
#include "utils/defs.h"
#include "compile/symb_table/NFunction.h"

#include "utils/TimerBench.h"
/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    []

  Description []

******************************************************************************/
typedef struct SymbTable_TAG
{
  SymbCache_ptr cache;
  hash_ptr categories;

  int temp_layer_name_suffix; /* used to create temporary names for layers */

  NodeList_ptr layers; /* the list of owned layers */

  hash_ptr class_layers; /* the list of layers organized by class */
  node_ptr class_names; /* the list of class names */
  const char* default_class_name; /* name of the default class name */

  hash_ptr name2layer; /* to associate layers and names */

  /* A counter for declaration of determinization variables */
  size_t det_counter;

  TypeChecker_ptr tc; /* this is the type checker owned by the ST */

  hash_ptr expr_simplify_hash; /* hash for function Expr_simplify */

  ResolveSymbol_ptr resolver;
} SymbTable;

/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define SYMB_TABLE_TEMP_LAYER_NAME "__TempLayer_%d"

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void symb_table_init ARGS((SymbTable_ptr self));
static void symb_table_deinit ARGS((SymbTable_ptr self));

static boolean
symb_table_layer_exists ARGS((const SymbTable_ptr self,
                              const char* layer_name));

static NodeList_ptr
symb_table_filter_layers_symbols ARGS((SymbTable_ptr self,
                                       const array_t* layer_names,
                                       SymbTableIter* iter));

static void
symb_table_layer_rename_in_class ARGS((SymbTable_ptr self,
                                       const char* class_name,
                                       const char* old_layer_name,
                                       const char* new_layer_name));

static array_t*
symb_table_get_layers_from_class ARGS((const SymbTable_ptr self,
                                       const char* class_name));

static array_t*
symb_table_create_layers_class ARGS((const SymbTable_ptr self,
                                     const char* class_name));

static array_t*
symb_table_get_layers_class ARGS((const SymbTable_ptr self,
                                  const char* class_name));

static node_ptr
symb_table_flatten_array_define ARGS((const SymbTable_ptr self,
                                      const node_ptr body,
                                      const node_ptr context));

static SymbCategory
symb_table_detect_expr_category ARGS((const SymbTable_ptr st,
                                       const Expr_ptr expr));
/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Class constructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbTable_ptr SymbTable_create()
{
  SymbTable_ptr self = ALLOC(SymbTable, 1);

  SYMB_TABLE_CHECK_INSTANCE(self);

  symb_table_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [Class destructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbTable_destroy(SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);

  symb_table_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Returns the internally stored type checker]

  Description        [Returned instance belongs to self]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
TypeChecker_ptr SymbTable_get_type_checker(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return self->tc;
}


/**Function********************************************************************

  Synopsis           [Creates and adds a new layer]

  Description [The created layer is returned. Do not destroy the
  layer, since it belongs to self. if layer name is NULL, then a
  temporary name will be searched and a new layer will be created. To
  retrieve the layer name, query the returned SymbLayer instance. ]

  SideEffects        []

  SeeAlso            [remove_layer]

******************************************************************************/
SymbLayer_ptr SymbTable_create_layer(SymbTable_ptr self,
                                     const char* layer_name,
                                     const LayerInsertPolicy ins_policy)
{
  SymbLayer_ptr layer;
  ListIter_ptr iter;
  char tmp_name[50];

  SYMB_TABLE_CHECK_INSTANCE(self);

  if (layer_name == (char*) NULL) {
    int c = snprintf(tmp_name, sizeof(tmp_name) / sizeof(tmp_name[0]),
                     SYMB_TABLE_TEMP_LAYER_NAME, self->temp_layer_name_suffix);
    SNPRINTF_CHECK(c, sizeof(tmp_name) / sizeof(tmp_name[0]));

    ++(self->temp_layer_name_suffix);
    layer_name = tmp_name;
  }

  nusmv_assert(! symb_table_layer_exists(self, layer_name));
  layer = SymbLayer_create(layer_name, ins_policy, self->cache);

  /* searches the insertion point, and inserts the layer */
  iter = NodeList_get_first_iter(self->layers);
  while (!ListIter_is_end(iter)) {
    if (SymbLayer_must_insert_before(layer,
             SYMB_LAYER(NodeList_get_elem_at(self->layers, iter)))) {
      NodeList_insert_before(self->layers, iter, (node_ptr) layer);
      break;
    }

    iter = ListIter_get_next(iter);
  }

  /* if not inserted yet: */
  if (ListIter_is_end(iter)) NodeList_append(self->layers, (node_ptr) layer);

  /* we duplicate the key here, to allow the caller to free layer_name
     if dinamically created. Memory will be freed by the deiniter of
     SymbTable */
  insert_assoc(self->name2layer, (node_ptr) find_string((char*) layer_name),
               (node_ptr) layer);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbTable: created new layer '%s'\n",  layer_name);
  }

  return layer;
}


/**Function********************************************************************

  Synopsis           [Removes and destroys a layer]

  Description [The layer must be not in use by any encoding, so remove
  it from all encodings before calling this method. The removed layer
  will be no longer available after the invocation of this method.

  If given layer belongs to a set of layer classes, the layer will
  be removed from the classes as well (meaning that there is no
  need to remove the layer from the classes it belongs to) ]

  SideEffects        []

  SeeAlso            [create_layer]

******************************************************************************/
void SymbTable_remove_layer(SymbTable_ptr self, SymbLayer_ptr layer)
{
  ListIter_ptr iter;

  SYMB_TABLE_CHECK_INSTANCE(self);
  nusmv_assert(symb_table_layer_exists(self, SymbLayer_get_name(layer)));

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbTable: destroying layer '%s'\n",
            SymbLayer_get_name(layer));
  }

  { /* removes the layer from all existing layers' classes: */
    node_ptr iter;
    for (iter=self->class_names; iter != Nil; iter = cdr(iter)) {
      const char* class_name = (const char*) car(iter);
      symb_table_layer_rename_in_class(self, class_name,
                                       SymbLayer_get_name(layer),
                                       (const char*) NULL);
    }
  }

  /* searches the layer */
  iter = NodeList_get_first_iter(self->layers);
  while (!ListIter_is_end(iter)) {
    SymbLayer_ptr lay;
    lay = SYMB_LAYER(NodeList_get_elem_at(self->layers, iter));
    if (layer == lay) {
      /* found the layer */
      NodeList_remove_elem_at(self->layers, iter);

      insert_assoc(self->name2layer,
                   (node_ptr) find_string((char*) SymbLayer_get_name(layer)),
                   (node_ptr) NULL);
      SymbLayer_destroy(layer);
      /* free the expression simplification hash. See info in Expr_simplify*/
      clear_assoc(self->expr_simplify_hash);
      return;
    }

    iter = ListIter_get_next(iter);
  }

  nusmv_assert(!ListIter_is_end(iter)); /* This layer had not been found */
}


/**Function********************************************************************

  Synopsis          [Given its name, returns a layer]

  Description [NULL is returned when the layer does not exist within
  self.  Returned SymbLayer instance belongs to self.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbLayer_ptr
SymbTable_get_layer(const SymbTable_ptr self, const char* layer_name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);

  /* If the name is null, then avoid the find_string */
  if ((char*)NULL == layer_name) {
    return SYMB_LAYER(NULL);
  }

  return SYMB_LAYER( find_assoc(self->name2layer,
                                (node_ptr) find_string((char*) layer_name)) );
}



/**Function********************************************************************

  Synopsis           [Renames an existing layer]

  Description [Use to rename an existing layer. Useful for example to
  substitute an existing layer with another.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbTable_rename_layer(const SymbTable_ptr self,
                            const char* layer_name, const char* new_name)
{
  SymbLayer_ptr layer;

  SYMB_TABLE_CHECK_INSTANCE(self);

  layer = SymbTable_get_layer(self, layer_name);
  SYMB_LAYER_CHECK_INSTANCE(layer);

  { /* renames the layer from all existing layers' classes: */
    node_ptr iter;
    for (iter=self->class_names; iter != Nil; iter = cdr(iter)) {
      const char* class_name = (const char*) car(iter);
      symb_table_layer_rename_in_class(self, class_name,
                                       layer_name, new_name);
    }
  }

  /* sets the new name */
  SymbLayer_set_name(layer, new_name);

  /* removes previous name association */
  insert_assoc(self->name2layer, (node_ptr) find_string((char*) layer_name),
               (node_ptr) NULL);

  /* adds the new name association */
  insert_assoc(self->name2layer, (node_ptr) find_string((char*) new_name),
               (node_ptr) layer);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbTable: renamed layer '%s' to '%s'\n",
            layer_name, new_name);
  }
}


/**Function********************************************************************

  Synopsis           [Returns the list of owned layers.]

  Description        [The returned list belongs to self. Do not free or
                      change it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr SymbTable_get_layers(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return self->layers;
}


/**Function********************************************************************

  Synopsis [Returns the default layers class name that has been
  previously set. The default layers class name is the class of
  layers that is taken when the system needs a default set of
  layers to work with. Typically the default class is the class of
  model layers, that is used for example when dumping the
  hierarchy by command write_bool_model.]

  Description        [Given string is duplicated.]

  SideEffects        []

  SeeAlso            [SymbTable_get_default_layers_class_name]

******************************************************************************/
void SymbTable_set_default_layers_class_name(SymbTable_ptr self,
                                             const char* class_name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);

  if (self->default_class_name != (const char*) NULL) {
    FREE(self->default_class_name);
    self->default_class_name = (const char*) NULL;
  }
  if (class_name != (const char*) NULL) {
    self->default_class_name = util_strsav((char*) class_name);
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
      fprintf(nusmv_stdout, "SymbTable: default layers class set to '%s'\n",
              class_name);
    }
  }
}

/**Function********************************************************************

  Synopsis [Returns the default layers class name that has been
  previously set. The default layers class name is the class of
  layers that is taken when the system needs a default set of
  layers to work with. Typically the default class is the class of
  model layers, that is used for example when dumping the
  hierarchy by command write_bool_model.]

  Description        [Returned string belongs to self, and must be NOT
  destroyed or changed. Returned string is NULL if not previously set.]

  SideEffects        []

  SeeAlso            [SymbTable_set_default_layers_class_name]

******************************************************************************/
const char* SymbTable_get_default_layers_class_name(const SymbTable_ptr self)
{ return self->default_class_name; }


/**Function********************************************************************

  Synopsis [Declares a new class of layers]

  Description [This method creates a new class of layers. The
  class must be not existing. The method can be used to create a
  class of layers that might be empty. It is not required to
  create a class before calling methods that use that class, like
  e.g.  SymbTable_layer_add_to_class that wll create the class
  when not existing. class_name can be NULL to create the default
  class (whose name must have been previously specified with
  SymbTable_set_default_layers_class_name)]

  SideEffects        []

  SeeAlso            [SymbTable_layer_add_to_class]

******************************************************************************/
void SymbTable_create_layer_class(SymbTable_ptr self, const char* class_name)
{
  array_t* _class;

  SYMB_TABLE_CHECK_INSTANCE(self);
  _class = symb_table_create_layers_class(self, class_name);
  nusmv_assert(_class != (array_t*) NULL);
  return;
}


/**Function********************************************************************

  Synopsis [Checks if a class of layers exists]

  Description [This method checks if class 'class_name' has been
  previously created in the SymbTable.Returns true if the class exists,
  false otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean  SymbTable_layer_class_exists(SymbTable_ptr self, const char* class_name)
{
  array_t* _class;

  SYMB_TABLE_CHECK_INSTANCE(self);
  _class = symb_table_get_layers_class(self, class_name);

  return ((array_t*) NULL != _class);
}


/**Function********************************************************************

  Synopsis [Adds a given layer (that must exist into self already)
  to a class of layers. Classes are used to group layers into
  possibly overlapping sets. For example the class of layers
  containing the set of symbols that belongs to the SMV model.  If
  class_name is NULL, the default class name will be taken (must
  be set before)]

  Description [A new class will be created if given class does not
  exist yet. The given layer must be existing.]

  SideEffects        []

  SeeAlso            [SymbTable_layer_remove_from_class]

******************************************************************************/
void SymbTable_layer_add_to_class(SymbTable_ptr self,
                                  const char* layer_name,
                                  const char* class_name)
{
  array_t* _class;

  SYMB_TABLE_CHECK_INSTANCE(self);
  nusmv_assert(symb_table_layer_exists(self, layer_name));

  _class = symb_table_create_layers_class(self, class_name);

  { /* checks that the class does not contain the layer already */
    const char* name; int i;
    arrayForEachItem(const char*, _class, i, name) {
      if (strcmp(name, layer_name) == 0) return;
    }
  }

  /* adds to the array */
  array_insert_last(const char*, _class, util_strsav((char*) layer_name));

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stdout, "SymbTable: Added layer '%s' to class ", layer_name);

    if ((char*)NULL != class_name) {
      fprintf(nusmv_stdout, "'%s'\n", class_name);
    }
    else {
      const char* def = SymbTable_get_default_layers_class_name(self);
      nusmv_assert((char*)NULL != def);
      fprintf(nusmv_stdout, "'%s'\n", def);
    }

  }
}


/**Function********************************************************************

  Synopsis           [Removes a given layer (that must exist into self already)
  from a given class of layers. If class_name is NULL, the default class
  is taken (must be set before)]

  Description [Given class must be existing, or if NULL default
  class must be existing. If the layer is not found, nothing happens.]

  SideEffects        []

  SeeAlso            [SymbTable_layer_add_to_class]

******************************************************************************/
void SymbTable_layer_remove_from_class(SymbTable_ptr self,
                                       const char* layer_name,
                                       const char* class_name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  symb_table_layer_rename_in_class(self, class_name,
                                   layer_name, (const char*) NULL);
}


/**Function********************************************************************

  Synopsis [Returns an array of layer names that belong to the
  given class name. If class_name is NULL, default class name will
  be taken (must be set before).]

  Description [Specified class must be existing, or if NULL is
  specified a default class must have been defined. Returned
  array belongs to self and has NOT to be destroyed or changed by
  the caller.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
array_t* SymbTable_get_class_layer_names(SymbTable_ptr self,
                                         const char* class_name)
{
  array_t* res;

  SYMB_TABLE_CHECK_INSTANCE(self);

  res = symb_table_get_layers_from_class(self, class_name);
  nusmv_assert(res != (array_t*) NULL);
  return res;
}


/**Function********************************************************************

  Synopsis    [Returns true if given layer name belongs to the given class]

  Description [If class_name is NULL, the default class will be checked]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbTable_is_layer_in_class(SymbTable_ptr self,
                                    const char* layer_name,
                                    const char* class_name)
{
  array_t* lays;
  const char* name;
  int i;

  SYMB_TABLE_CHECK_INSTANCE(self);

  lays = SymbTable_get_class_layer_names(self, class_name);
  arrayForEachItem(const char*, lays, i, name) {
    if (strcmp(name, layer_name) == 0) return true;
  }

  return false;
}


/**Function********************************************************************

  Synopsis    [Returns the name of the class in which the given layer is
               declared or NULL if there is no such a class.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
const char* SymbTable_get_class_of_layer(const SymbTable_ptr self,
                                         const char* layer_name)
{
  node_ptr iter;

  SYMB_TABLE_CHECK_INSTANCE(self);

  for (iter=self->class_names; iter != Nil; iter = cdr(iter)) {
    const char* class_name;

    class_name = (const char*) car(iter);
    if (SymbTable_is_layer_in_class(self, layer_name, class_name)) {
      return class_name;
    }
  }

  return (const char*) NULL;
}


/**Function********************************************************************

  Synopsis           [Returns the number of all declared variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_vars_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_state_vars_num(self->cache) +
    SymbCache_get_input_vars_num(self->cache) +
    SymbCache_get_frozen_vars_num(self->cache);
}


/**Function********************************************************************

  Synopsis           [Returns the number of all declared state variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_state_vars_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_state_vars_num(self->cache);
}


/**Function********************************************************************

  Synopsis           [Returns the number of all declared frozen variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_frozen_vars_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_frozen_vars_num(self->cache);
}


/**Function********************************************************************

  Synopsis           [Returns the number of all declared input variables]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_input_vars_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_input_vars_num(self->cache);
}


/**Function********************************************************************

  Synopsis           [Returns the number of all declared constants]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_constants_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_constants_num(self->cache);
}

/**Function********************************************************************

  Synopsis           [Returns the number of all declared defines]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_defines_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_defines_num(self->cache);
}

/**Function********************************************************************

  Synopsis           [Returns the number of all declared array define]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_array_defines_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_array_defines_num(self->cache);
}

/**Function********************************************************************

  Synopsis           [Returns the number of all parameters]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_parameters_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_parameters_num(self->cache);
}

/**Function********************************************************************

  Synopsis           [Returns the number of all NFunctions]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int SymbTable_get_functions_num(const SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_functions_num(self->cache);
}

/**Function********************************************************************

  Synopsis           [Returns the list of state and frozen variables
  that belong to the given layers]

  Description        [Everytime this method is called, it will create and
  calculate a new list. layers is an array of strings.
  WARNING: The returned instance must be destroyed by the caller]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr SymbTable_get_layers_sf_vars(SymbTable_ptr self,
                                          const array_t* layer_names)
{
  SymbTableIter iter;

  SYMB_TABLE_CHECK_INSTANCE(self);

  SymbTable_gen_iter(self, &iter, STT_STATE_VAR | STT_FROZEN_VAR);

  return symb_table_filter_layers_symbols(self, layer_names, &iter);
}


/**Function********************************************************************

  Synopsis           [Returns the list of state and frozen symbols
  that belong to the given layers]

  Description        [Everytime this method is called, it will create and
  calculate a new list. layers is an array of strings.
  WARNING: The returned instance must be destroyed by the caller.
  Note: state symbols include frozen variables.]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr SymbTable_get_layers_sf_symbols(SymbTable_ptr self,
                                             const array_t* layer_names)
{
  SymbTableIter iter;

  SYMB_TABLE_CHECK_INSTANCE(self);

  SymbTable_gen_iter(self, &iter, STT_ALL);
  SymbTable_iter_set_filter(self, &iter,
                            SymbTable_iter_filter_sf_symbols, NULL);

  return symb_table_filter_layers_symbols(self, layer_names, &iter);
}


/**Function********************************************************************

  Synopsis           [Returns the list of input symbols that belong to the
  given layers]

  Description        [Everytime this method is called, it will create and
  calculate a new list. layers is an array of strings.
  WARNING: The returned instance must be destroyed by the caller]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr SymbTable_get_layers_i_symbols(SymbTable_ptr self,
                                            const array_t* layer_names)
{
  SymbTableIter iter;

  SYMB_TABLE_CHECK_INSTANCE(self);

  SymbTable_gen_iter(self, &iter, STT_ALL);
  SymbTable_iter_set_filter(self, &iter,
                            SymbTable_iter_filter_i_symbols, NULL);

  return symb_table_filter_layers_symbols(self, layer_names, &iter);
}


/**Function********************************************************************

  Synopsis           [Returns the list of input variables that belong to the
  given layers]

  Description        [Everytime this method is called, it will create and
  calculate a new list. layers is an array of strings.
  WARNING: The returned instance must be destroyed by the caller]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr SymbTable_get_layers_i_vars(SymbTable_ptr self,
                                         const array_t* layer_names)
{
  SymbTableIter iter;

  SYMB_TABLE_CHECK_INSTANCE(self);

  SymbTable_gen_iter(self, &iter, STT_INPUT_VAR);

  return symb_table_filter_layers_symbols(self, layer_names, &iter);
}



/**Function********************************************************************

  Synopsis [Returns the list of state and input symbols that
  belong to the given layers, meaning those DEFINES whose body
  contain both state (or frozen) and input variables. This methods
  does _NOT_ return the state symbols plus the input symbols.]

  Description        [Everytime this method is called, it will create and
  calculate a new list. layers is an array of strings.
  WARNING: The returned instance must be destroyed by the caller]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr
SymbTable_get_layers_sf_i_symbols(SymbTable_ptr self,
                                  const array_t* layer_names)
{
  SymbTableIter iter;

  SYMB_TABLE_CHECK_INSTANCE(self);

  SymbTable_gen_iter(self, &iter, STT_ALL);
  SymbTable_iter_set_filter(self, &iter,
                            SymbTable_iter_filter_sf_i_symbols, NULL);

  return symb_table_filter_layers_symbols(self, layer_names, &iter);}


/**Function********************************************************************

  Synopsis [Returns the list of variables that belong to the given layers]

  Description        [Everytime this method is called, it will create and
  calculate a new list. layers is an array of strings.
  WARNING: The returned instance must be destroyed by the caller]

  SideEffects        []

  See Also           []

******************************************************************************/
NodeList_ptr
SymbTable_get_layers_sf_i_vars(SymbTable_ptr self,
                               const array_t* layer_names)
{
  SymbTableIter iter;

  SYMB_TABLE_CHECK_INSTANCE(self);

  SymbTable_gen_iter(self, &iter, STT_VAR);

  return symb_table_filter_layers_symbols(self, layer_names, &iter);
}


/**Function********************************************************************

  Synopsis           [Returns the type of a given variable]

  Description        [The type belongs to the layer, do not destroy it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbType_ptr
SymbTable_get_var_type(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SYMB_TYPE(SymbCache_get_var_type(self->cache, name));
}


/**Function********************************************************************

  Synopsis           [Returns the body of the given DEFINE]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_define_body(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_define_body(self->cache, name);
}

/**Function********************************************************************

  Synopsis           [Returns the actual param of the given formal parameter ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_actual_parameter(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_actual_parameter(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns the body of the given array define name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_array_define_body(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_array_define_body(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns the body of the given array define name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbType_ptr
SymbTable_get_variable_array_type(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_variable_array_type(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns the NFunction with the given name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NFunction_ptr
SymbTable_get_function(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_function(self->cache, name);
}

/**Function********************************************************************

  Synopsis           [Returns the context of the NFunction with the given name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_function_context(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_function_context(self->cache, name);
}

/**Function********************************************************************

  Synopsis           [Returns the flattened body of the given array define name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_array_define_flatten_body(const SymbTable_ptr self,
                                         const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);

  return symb_table_flatten_array_define(self,
                                    SymbCache_get_array_define_body(self->cache, name),
                                    SymbCache_get_array_define_context(self->cache, name));
}

/**Function********************************************************************

  Synopsis [Returns the flattenized body of the given
  define]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_define_flatten_body(const SymbTable_ptr self,
                                         const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_define_flatten_body(self->cache, name);
}

/**Function********************************************************************

  Synopsis [Returns the flattenized actual parameter of the given
  formal parameter]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_flatten_actual_parameter(const SymbTable_ptr self,
                                       const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_flatten_actual_parameter(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns the context of the given DEFINE name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_define_context(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_define_context(self->cache, name);
}

/**Function********************************************************************

  Synopsis [Returns the context of the actual parameter associated
  with the given formal one]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_actual_parameter_context(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_actual_parameter_context(self->cache, name);
}

/**Function********************************************************************

  Synopsis           [Returns the context of the given array define name]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr
SymbTable_get_array_define_context(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_get_array_define_context(self->cache, name);
}

/**Function********************************************************************

  Synopsis    [This function returns the category of
  an identifier]

  Description [Possible categories are: state vars, input vars, state
  only defines, input only defines, state-input defines,
  state-input-next defines. If a symbol is not properly recognized as
  one of the above, SYMBOL_INVALID is returned (for constants, for examples).

  An identifier is var or define. It is also allowed to have arrays
  with constant index, i.e. if V is identifier than V[5] is also
  identifier.]

  SideEffects [None]

******************************************************************************/
SymbCategory SymbTable_get_symbol_category(const SymbTable_ptr self,
                                           node_ptr symbol)
{
  SymbCategory res;

  SYMB_TABLE_CHECK_INSTANCE(self);

  /* 1. Handle memoized results */
  res = NODE_TO_INT(find_assoc(self->categories, symbol));
  if (SYMBOL_INVALID != res) return res;

 /* 2. Trying to detect category */
  if (SymbTable_is_symbol_constant(self, symbol)) {
    res = SYMBOL_CONSTANT;
  }
  else if (SymbTable_is_symbol_frozen_var(self, symbol)) {
    res = SYMBOL_FROZEN_VAR;
  }
  else if (SymbTable_is_symbol_state_var(self, symbol)) {
    res = SYMBOL_STATE_VAR;
  }
  else if (SymbTable_is_symbol_input_var(self, symbol)) {
    res = SYMBOL_INPUT_VAR;
  }
  else if (SymbTable_is_symbol_define(self, symbol)) {
    node_ptr def_flatten_body =
      SymbTable_get_define_flatten_body(self, symbol);
    res = symb_table_detect_expr_category(self, def_flatten_body);
  }
  else {
    /* it is not a var or define. the only remaining possibility is array.
       otherwise it is undefined symbol. */
    if (ARRAY == node_get_type(symbol)) {
      /* only constant indexes are allowed in identifiers */
      nusmv_assert(node_is_leaf(cdr(symbol)));
      res = SymbTable_get_symbol_category(self, car(symbol));
    }
    else {
      /* only identifiers are allowed */
      nusmv_assert(DOT == node_get_type(symbol) || ATOM == node_get_type(symbol));
      res = SYMBOL_INVALID;
    }
  }

  insert_assoc(self->categories, symbol, NODE_FROM_INT(res));

  return res;
}


/**Function********************************************************************

  Synopsis [Returns true if the given symbol is a state variable.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_state_var(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_state_var(self->cache, name);
}


/**Function********************************************************************

  Synopsis [Returns true if the given symbol is a frozen variable.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_frozen_var(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_frozen_var(self->cache, name);
}


/**Function********************************************************************

  Synopsis [Returns true if the given symbol is a frozen or a state variable.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_state_frozen_var(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_state_frozen_var(self->cache, name);
}


/**Function********************************************************************

  Synopsis [Returns true if the given symbol is an input variable.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_input_var(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_input_var(self->cache, name);
}


/**Function********************************************************************

  Synopsis [Returns true if the given symbol is either a state, frozen or
  an input variable.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_var(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_var(self->cache, name);
}


/**Function********************************************************************

  Synopsis [Returns true if the given symbol is a variable of enum type
  with the values 0 and 1 (boolean)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_bool_var(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);

  if (SymbCache_is_symbol_var(self->cache, name)) {
    SymbType_ptr type = SymbTable_get_var_type(self, name);
    return (SYMB_TYPE(NULL) != type) && SymbType_is_boolean(type);
  }

  return false;
}


/**Function********************************************************************

  Synopsis           [Returns true if the given symbol is declared]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_declared(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_declared(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given symbol is a declared
  DEFINE]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_define(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_define(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given symbol is a declared
                      NFunction]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_function(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_function(self->cache, name);
}

/**Function********************************************************************

  Synopsis           [Returns true if the given symbol is a declared
  parameter]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_parameter(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_parameter(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given symbol is a declared
  array define]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_array_define(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_array_define(self->cache, name);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given symbol is a declared
  variable array]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_variable_array(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_variable_array(self->cache, name);
}


/**Function********************************************************************

  Synopsis [Returns true if the given symbol is a declared
  constant]

  Description [Notice that this method will check only symbols defined
  within self. For example if an integer constant was not declared
  within self, this method will return false for it. For generic
  expressions, consider using function node_is_leaf which performs a
  purely-syntactly check.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_is_symbol_constant(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_is_symbol_constant(self->cache, name);
}


/**Function********************************************************************

  Synopsis [Returns true if the given variable has a finite domain]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbTable_is_var_finite(const SymbTable_ptr self, const node_ptr name)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return !SymbType_is_infinite_precision(SymbTable_get_var_type(self, name));
}



/**Function********************************************************************

  Synopsis           [Returns true if var_list contains at least one input
  variable, false otherwise]

  Description        [The given list of variables is traversed until an input
  variable is found]

  SideEffects        []

  See Also           []

******************************************************************************/
boolean
SymbTable_list_contains_input_var(const SymbTable_ptr self,
                                  const NodeList_ptr var_list)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_list_contains_input_var(self->cache, var_list);
}


/**Function********************************************************************

  Synopsis           [Returns true if var_list contains at least one state
  or frozen variable, false otherwise]

  Description        [The given list of variables is traversed until
  a state or frozen variable is found]

  SideEffects        []

  See Also           []

******************************************************************************/
boolean
SymbTable_list_contains_state_frozen_var(const SymbTable_ptr self,
                                         const NodeList_ptr var_list)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_list_contains_state_frozen_var(self->cache, var_list);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given symbols list contains
  one or more undeclared variable names, false otherwise]

  Description        [Iterates through the elements in var_list
  checking each one to see if it is one undeclared variable.]

  SideEffects        []

  See Also           []

******************************************************************************/
boolean
SymbTable_list_contains_undef_var(const SymbTable_ptr self,
                                  const NodeList_ptr var_list)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return SymbCache_list_contains_undef_var(self->cache, var_list);
}


/**Function********************************************************************

  Synopsis           [Returns the layer a variable is defined in.]

  Description        [Returns the layer a variable is defined in, NULL
  if there is no layer containing it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbLayer_ptr SymbTable_variable_get_layer(SymbTable_ptr  self, node_ptr name)
{
  NodeList_ptr layers;
  ListIter_ptr iter;
  SymbLayer_ptr layer, result = SYMB_LAYER(NULL);

  SYMB_TABLE_CHECK_INSTANCE(self);

  if (!SymbTable_is_symbol_var(self, name)) {
    return SYMB_LAYER(NULL);
  }

  layers = self->layers;

  NODE_LIST_FOREACH(layers, iter) {
    layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
    if (SymbLayer_is_symbol_in_layer(layer, name)) {
      result = layer;
      break;
    }
  }

  nusmv_assert(SYMB_LAYER(NULL) != result);

  return result;
}


/**Function********************************************************************

  Synopsis           [Returns the layer a symbol is defined in.]

  Description        [Returns the layer a symbol is defined in, NULL
                      if there is no layer containing it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbLayer_ptr SymbTable_symbol_get_layer(SymbTable_ptr  self, node_ptr name)
{
  NodeList_ptr layers;
  ListIter_ptr iter;
  SymbLayer_ptr layer, result = SYMB_LAYER(NULL);

  SYMB_TABLE_CHECK_INSTANCE(self);

  if (!SymbTable_is_symbol_declared(self, name)) {
    return SYMB_LAYER(NULL);
  }

  layers = self->layers;

  NODE_LIST_FOREACH(layers, iter) {
    layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
    if (SymbLayer_is_symbol_in_layer(layer, name)) {
      result = layer;
      break;
    }
  }

  nusmv_assert(SYMB_LAYER(NULL) != result);

  return result;
}


/**Function********************************************************************

  Synopsis           [Returns the layer a DEFINE is defined in.]

  Description        [Returns the layer a DEFINE is defined in, NULL
  if there is no layer containing it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbLayer_ptr SymbTable_define_get_layer(SymbTable_ptr  self, node_ptr name)
{
  NodeList_ptr layers;
  ListIter_ptr iter;
  SymbLayer_ptr layer, result = SYMB_LAYER(NULL);

  SYMB_TABLE_CHECK_INSTANCE(self);

  if (!SymbTable_is_symbol_define(self, name)) {
    return SYMB_LAYER(NULL);
  }

  layers = self->layers;

  NODE_LIST_FOREACH(layers, iter) {
    layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
    if (SymbLayer_is_symbol_in_layer(layer, name)) {
      result = layer;
      break;
    }
  }

  nusmv_assert(SYMB_LAYER(NULL) != result);

  return result;
}

/**Function********************************************************************

  Synopsis           [Returns the layer a NFunction is defined in.]

  Description        [Returns the layer a NFunction is defined in, NULL
                      if there is no layer containing it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
SymbLayer_ptr SymbTable_function_get_layer(SymbTable_ptr self, node_ptr name)
{
  NodeList_ptr layers;
  SymbLayer_ptr layer, result = SYMB_LAYER(NULL);
  ListIter_ptr iter;

  SYMB_TABLE_CHECK_INSTANCE(self);

  if (!SymbTable_is_symbol_function(self, name)) {
    return SYMB_LAYER(NULL);
  }

  layers = self->layers;

  NODE_LIST_FOREACH(layers, iter) {
    layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
    if (SymbLayer_is_symbol_in_layer(layer, name)) {
      result = layer;
      break;
    }
  }

  nusmv_assert(SYMB_LAYER(NULL) != result);

  return result;
}


/**Function********************************************************************

  Synopsis           [Returns a valid name for a new determinization variable]

  Description        [Returns a valid name for a new determinization
                      variable.  Searches in the symbol table for a
                      variable name which is not declared yet, and
                      returns it. Warning: This method does not
                      declare a new variable, it simply finds a
                      valid name for a new determinization
                      variable. If the returned variable name is
                      not used later to declare a new variable,
                      succeed calls to this method may not return a
                      valid name.]

  SideEffects        []

  SeeAlso            [symb_table_deinit]

******************************************************************************/
node_ptr SymbTable_get_determinization_var_name(const SymbTable_ptr self)
{
  const char* INT_VAR_PREFIX = "__det_";
  char* name = (char*) NULL;
  node_ptr vname = Nil;
  int buf_size = strlen(INT_VAR_PREFIX) + 10;

  SYMB_TABLE_CHECK_INSTANCE(self);

  /* 10 digits should be enough */
  nusmv_assert(self->det_counter < (size_t) (unsigned int) ~0);
  name = (char*) ALLOC(char, buf_size);

  /* searches for a not already declared symbol for the new determ var: */
  while (true) {
    int c = snprintf(name, buf_size, "%s%" PRIuPTR,
                     INT_VAR_PREFIX, self->det_counter++);
    SNPRINTF_CHECK(c, buf_size);
    vname = find_node(DOT, Nil, sym_intern(name));
    if (!SymbTable_is_symbol_declared(self, vname)) break;
  }

  FREE(name);
  return vname;
}


/**Function********************************************************************

  Synopsis           [Given a tplate, returns a fresh symbol name.  This
                      function NEVER returns the same symbol twice and
                      NEVER returns a declared name]

  Description        [If tplate is NULL then a valid fresh symbol is choosed.
  NB: here tplate actually means prefix.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr SymbTable_get_fresh_symbol_name(SymbTable_ptr self,
                                         const char* tplate) {
  static int i = 0;
  int size;
  char* varname;
  node_ptr res;

  if ((char*)NULL == tplate) {
    tplate = "__fresh_var_";
  }

  /* 15 digits are wide enough for any reasonable number */
  size = strlen(tplate) + 16;
  varname = ALLOC(char, size);

  do {
    snprintf(varname, size, "%s%d", tplate, i);
    i++;
    res = find_node(DOT, Nil, find_node(ATOM, (node_ptr) find_string(varname), Nil));
  } while(SymbTable_is_symbol_declared(self, res));

  FREE(varname);

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns a hash which can be used only in
  Expr_simplify.]

  Description        [See function Expr_simplify for more details. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
hash_ptr SymbTable_get_simplification_hash(SymbTable_ptr self)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  return self->expr_simplify_hash;
}


/**Function********************************************************************

  Synopsis           [Create a new SymbolTable which contains the same info
                      as the given one except the specified symbols in blacklist]

  Description        [Returned ST is allocated and has to be released by caller.
                      The copy is performed iterating over each layer in the
                      Symbol Table. The new ST contains a copy of each layer of
                      the given Symbol Table]

  SideEffects        []

  SeeAlso            [SymbTable_create SymbTable_destroy]

******************************************************************************/
SymbTable_ptr SymbTable_copy(const SymbTable_ptr self, Set_t blacklist)
{
  SymbTable_ptr new_st;
  SymbLayer_ptr new_layer, old_layer;
  ListIter_ptr layer_iter;
  NodeList_ptr layers;

  new_st = SymbTable_create();

  SymbTable_set_default_layers_class_name(new_st,
                                SymbTable_get_default_layers_class_name(self));

  layers = SymbTable_get_layers(self);
  NODE_LIST_FOREACH(layers, layer_iter) {
    const char* class_name;
    SymbLayerIter iter;

    old_layer = (SymbLayer_ptr)NodeList_get_elem_at(layers, layer_iter);

    new_layer = SymbTable_create_layer(new_st,
                                       SymbLayer_get_name(old_layer),
                                       SymbLayer_get_insert_policy(old_layer));

    class_name = SymbTable_get_class_of_layer(self,
                                              SymbLayer_get_name(old_layer));
    if ((char*)NULL != class_name) {
      SymbTable_layer_add_to_class(new_st, SymbLayer_get_name(old_layer),
                                   class_name);
    }

    /* SYMB_LAYER_FOREACH(old_layer, iter, STT_CONSTANT) { */
    /*   node_ptr symbol = SymbLayer_iter_get_symbol(old_layer, &iter); */
    /*   SymbLayer_declare_constant(new_layer, symbol); */
    /* } */

    SYMB_LAYER_FOREACH(old_layer, iter, STT_ALL) {
      node_ptr symbol = SymbLayer_iter_get_symbol(old_layer, &iter);

      /* Skip blacklisted symbols*/
      if (!Set_IsMember(blacklist, symbol)) {
        /* We have different approaches for variables and symbols */
        if (SymbTable_is_symbol_var(self, symbol)) {
          SymbType_ptr type = SymbTable_get_var_type(self, symbol);

          if (SymbType_is_enum(type)) {
            node_ptr value_list = SymbType_get_enum_type_values(type);

            while (value_list != Nil) {
              node_ptr name = car(value_list);

              if (DOT == node_get_type(name)) {
                ResolveSymbol_ptr rs;
                rs = SymbTable_resolve_symbol(self, name, Nil);
                name = ResolveSymbol_get_resolved_name(rs);
              }
              else name = find_atom(name);

              if (SymbLayer_can_declare_constant(new_layer, name)) {
                SymbLayer_declare_constant(new_layer, name);
              }
              value_list = cdr(value_list);
            }
          } /* SymbType_is_enum */

          /* Create a copy of the type */
          type = SymbType_copy(type);

          /* Add the variable to the new symbol layer */
          if (SymbTable_is_symbol_state_var(self, symbol)) {
            SymbLayer_declare_state_var(new_layer, symbol, type);
          }
          else if (SymbTable_is_symbol_frozen_var(self, symbol)) {
            SymbLayer_declare_frozen_var(new_layer, symbol, type);
          }
          else if (SymbTable_is_symbol_input_var(self, symbol)) {
            SymbLayer_declare_input_var(new_layer, symbol, type);
          }
          else {
            rpterr("SymbTable_copy: Symbol %s not handled.\n",
                   sprint_node(symbol));
            error_unreachable_code();
          }
        }
        /* Not variables */
        else {
          if (SymbTable_is_symbol_constant(self, symbol)) {
            /* Constants should have been already declared. */
            SymbLayer_declare_constant(new_layer, symbol);
          }
          else if (SymbTable_is_symbol_variable_array(self, symbol)) {
            SymbType_ptr type = SymbTable_get_variable_array_type(self, symbol);
            type = SymbType_copy(type);
            SymbLayer_declare_variable_array(new_layer, symbol, type);
          }
          else if (SymbTable_is_symbol_array_define(self, symbol)) {
            node_ptr body =
              SymbTable_get_array_define_flatten_body(self, symbol);

            /* If we remove this function call, when someone asks the
               body of the define in the new symbol table, the old body
               is returned (eg: WriteFlattenModel) */
            Flatten_remove_symbol_info(symbol);

            SymbLayer_declare_array_define(new_layer, symbol, Nil, body);
          }
          else if (SymbTable_is_symbol_define(self, symbol)) {
            node_ptr body = SymbTable_get_define_body(self, symbol);
            node_ptr ctx = SymbTable_get_define_context(self, symbol);

            /* If we remove this function call, when someone asks the
               body of the define in the new symbol table, the old body
               is returned (eg: WriteFlattenModel) */
            Flatten_remove_symbol_info(symbol);

            SymbLayer_declare_define(new_layer, symbol, ctx, body);
          }
          else if (SymbTable_is_symbol_parameter(self, symbol)) {
            node_ptr actual = SymbTable_get_actual_parameter(self, symbol);
            node_ptr ctx = SymbTable_get_actual_parameter_context(self, symbol);

            SymbLayer_declare_parameter(new_layer, symbol, ctx, actual);
          }
          else {
            char* n = sprint_node(symbol);
            rpterr("SymbTable_copy: Symbol %s not handled.\n", n);
            FREE(n);
            error_unreachable_code();
          }
        }
      }
    }
  }
  return new_st;
}

/**Function********************************************************************

  Synopsis           [Resolves the given symbol in the given context]

  Description        [Resolves the given symbol in the given context.
                      This function returns the internal instance of
                      ResolveSymbol, which must NOT be freed by the
                      caller. The ResolveSymbol internal instance is
                      re-populated everytime that this function is
                      called.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ResolveSymbol_ptr SymbTable_resolve_symbol(SymbTable_ptr self,
                                           node_ptr expr, node_ptr context)
{
  SYMB_TABLE_CHECK_INSTANCE(self);
  ResolveSymbol_resolve(self->resolver, self, expr, context);
  return self->resolver;
}

/**Function********************************************************************

  Synopsis           [Initializes the given iterator with the given mask.]

  Description        [Initializes the given iterator with the given mask.
                      It is fundamental to call this procedure before
                      using the iterator.

                      SYMB_TABLE_FOREACH and SYMB_TABLE_FOREACH_FILTER
                      automaticcaly call this function, so the caller
                      does not have to worry about that.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbTable_gen_iter(const SymbTable_ptr self,
                        SymbTableIter* iter,
                        unsigned int mask)
{
  SymbCache_gen_iter(self->cache, iter, mask);
}

/**Function********************************************************************

  Synopsis           [Moves the iterator to the next valid symbol]

  Description        [Moves the iterator to the next valid symbol]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbTable_iter_next(const SymbTable_ptr self, SymbTableIter* iter)
{
  SymbCache_next_iter(self->cache, iter);
}

/**Function********************************************************************

  Synopsis           [Checks if the iterator is at it's end]

  Description        [Checks if the iterator is at it's end]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbTable_iter_is_end(const SymbTable_ptr self,
                              const SymbTableIter* iter)
{
  return SymbCache_is_iter_end(self->cache, iter);
}


/**Function********************************************************************

  Synopsis           [Gets the symbol pointed by the given iterator]

  Description        [Gets the symbol pointed by the given iterator.
                      The given iterator must not be at it's end]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr SymbTable_iter_get_symbol(const SymbTable_ptr self,
                                   const SymbTableIter* iter)
{
  return SymbCache_iter_get_symbol(self->cache, iter);
}



/**Function********************************************************************

  Synopsis           [Sets the filter to be used by the iterator]

  Description        [Sets the filter to be used by the iterator.
                      The iterator internally moves itself to the next
                      valid symbol that satisfies both the filter and
                      the mask]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbTable_iter_set_filter(const SymbTable_ptr self,
                               SymbTableIter* iter,
                               SymbTableIterFilterFun fun,
                               void* arg)
{
  iter->st = self;
  SymbCache_iter_set_filter(self->cache, iter, fun, arg);
}


/**Function********************************************************************

  Synopsis           [Default iterator filter: State, Frozen and Input symbols]

  Description        [Default iterator filter: State, Frozen and Input symbols.
                      Only defines that predicate over state or frozen
                      AND input variables or variables themselfs
                      satisfy this filter.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbTable_iter_filter_sf_i_symbols(const SymbTable_ptr self,
                                           const node_ptr sym,
                                           void* arg)
{
  if (SymbTable_is_symbol_define(self, sym)) {
    SymbCategory cat = SymbTable_get_symbol_category(self, sym);
    return cat == SYMBOL_STATE_INPUT_NEXT_DEFINE ||
      cat == SYMBOL_STATE_INPUT_DEFINE;
  }

  return SymbTable_is_symbol_var(self, sym);
}


/**Function********************************************************************

  Synopsis           [Default iterator filter: State, Frozen symbols]

  Description        [Default iterator filter: State, Frozen symbols.
                      Only defines that predicate over state or frozen
                      variables or state / frozen variables themselfs
                      satisfy this filter.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbTable_iter_filter_sf_symbols(const SymbTable_ptr self,
                                         const node_ptr sym,
                                         void* arg)
{
  if (SymbTable_is_symbol_define(self, sym)) {
    SymbCategory cat = SymbTable_get_symbol_category(self, sym);
    return cat == SYMBOL_STATE_NEXT_DEFINE ||
      cat == SYMBOL_STATE_DEFINE ||
      cat == SYMBOL_CONSTANT;
  }

  return SymbTable_is_symbol_state_var(self, sym) ||
    SymbTable_is_symbol_frozen_var(self, sym);
}


/**Function********************************************************************

  Synopsis           [Default iterator filter: Input symbols]

  Description        [Default iterator filter: Input symbols.
                      Only defines that predicate over input variables
                      or input variables themselfs satisfy this
                      filter.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean SymbTable_iter_filter_i_symbols(const SymbTable_ptr self,
                                        const node_ptr sym,
                                        void* arg)
{
  if (SymbTable_is_symbol_define(self, sym)) {
    SymbCategory cat = SymbTable_get_symbol_category(self, sym);
    return cat == SYMBOL_INPUT_NEXT_DEFINE ||
      cat == SYMBOL_INPUT_DEFINE;
  }

  return SymbTable_is_symbol_input_var(self, sym);
}


/**Function********************************************************************

  Synopsis           [Executes the given function over each symbol
                      that satisfies the given symbol mask]

  Description        [Executes the given function over each symbol
                      that satisfies the given symbol mask]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SymbTable_foreach(const SymbTable_ptr self, unsigned int mask,
                       SymbTableForeachFun fun, void* arg)
{
  SymbTableIter iter;

  SYMB_TABLE_FOREACH(self, iter, mask) {
    fun(self, SymbCache_iter_get_symbol(self->cache, &iter), arg);
  }
}

/**Function********************************************************************

  Synopsis           [Creates a set starting from the iterator]

  Description        [Creates a set starting from the iterator. The set
                      must be freed. The iterator is NOT changed (it
                      is passed as value..)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t SymbTable_iter_to_set(const SymbTable_ptr self,
                            SymbTableIter iter)
{
  Set_t res = Set_MakeEmpty();

  while (!SymbTable_iter_is_end(self, &iter)) {
    res = Set_AddMember(res, SymbTable_iter_get_symbol(self, &iter));
    SymbTable_iter_next(self, &iter);
  }

  return res;
}

/**Function********************************************************************

  Synopsis           [Creates a set starting from the iterator]

  Description        [Creates a list starting from the iterator. The list
                      must be freed. The iterator is NOT changed (it
                      is passed as value..)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr SymbTable_iter_to_list(const SymbTable_ptr self,
                                    SymbTableIter iter)
{
  NodeList_ptr res = NodeList_create();

  while (!SymbTable_iter_is_end(self, &iter)) {
    NodeList_append(res, SymbTable_iter_get_symbol(self, &iter));
    SymbTable_iter_next(self, &iter);
  }

  return res;
}

/**Function********************************************************************

  Synopsis           [Counts the elements of the iterator]

  Description        [Counts the elements of the iterator. The iterator
                      is NOT changed (it is passed as value..)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
unsigned int SymbTable_iter_count(const SymbTable_ptr self,
                                  SymbTableIter iter)
{
  unsigned int res = 0;

  while (!SymbTable_iter_is_end(self, &iter)) {
    ++res;
    SymbTable_iter_next(self, &iter);
  }

  return res;
}

/**Function********************************************************************

  Synopsis           [Adds a trigger to the symbol table]

  Description        [Adds a trigger to the symbol table.
                      "arg" is the argument that will be passed to
                      function "trigger" when invoked.

                      The "action" parameter determines when "trigger"
                      is triggered. The possibilities are:

                      ST_TRIGGER_SYMBOL_ADD: Triggered when a symbol
                        is added. When the trigger is called, all
                        informations about the symbol are already
                        available (e.g. SymbType).

                      ST_TRIGGER_SYMBOL_REMOVE: Triggered when a
                        symbol is removed. This may happen when
                        removing a layer from the symbol table, or
                        when removing a variable from a layer. All
                        informations about the symbol are still
                        available when the trigger is invoked

                      ST_TRIGGER_SYMBOL_REDECLARE: Triggered when a
                        symbol that had been removed from the symbol
                        table, or a from a layer, is redeclared with
                        the same name. All informations about the new
                        symbol are available, while informations about
                        the old symbol are not]

  SideEffects        []

  SeeAlso            [SymbTable_remove_trigger]

******************************************************************************/
void SymbTable_add_trigger(const SymbTable_ptr self,
                           SymbTableTriggerFun trigger,
                           SymbTableTriggerAction action,
                           void* arg)
{
  SymbCache_add_trigger(self->cache, trigger, action, arg);
}

/**Function********************************************************************

  Synopsis           [Removes a trigger from the Symbol Table]

  Description        [Removes a trigger from the Symbol Table]

  SideEffects        []

  SeeAlso            [SymbTable_add_trigger]

******************************************************************************/
void SymbTable_remove_trigger(const SymbTable_ptr self,
                              SymbTableTriggerFun trigger,
                              SymbTableTriggerAction action)
{
  SymbCache_remove_trigger(self->cache, trigger, action);
}


/**Function********************************************************************

  Synopsis           [Checks whether the Symbol Table contains infinite
                      precision variables]

  Description        [Checks whether the Symbol Table contains infinite
                      precision variables]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean
SymbTable_contains_infinite_precision_variables(const SymbTable_ptr self)
{
  SymbTableIter iter;

  SYMB_TABLE_FOREACH(self, iter, STT_VAR) {
    node_ptr sym = SymbCache_iter_get_symbol(self->cache, &iter);
    SymbType_ptr type = SymbTable_get_var_type(self, sym);

    if (SymbType_is_infinite_precision(type)) {
      return true;
    }
  }

  return false;
}

/*--------------------------------------------------------------------------*/
/* Definition of static functions                                           */
/*--------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private initializer]

  Description        [Private initializer, called by the constructor only]

  SideEffects        []

  SeeAlso            [symb_table_deinit]

******************************************************************************/
static void symb_table_init(SymbTable_ptr self)
{
  self->cache = SymbCache_create(self);
  self->categories = new_assoc();
  self->temp_layer_name_suffix = 0;
  self->layers = NodeList_create();
  self->class_layers = new_assoc();
  nusmv_assert(self->class_layers != (hash_ptr) NULL);
  self->class_names = Nil;
  self->default_class_name = (const char*) NULL;

  self->name2layer = new_assoc();
  nusmv_assert(self->name2layer != (hash_ptr) NULL);

  self->det_counter = 0;
  self->tc = TypeChecker_create_with_default_checkers(self);

  self->expr_simplify_hash = new_assoc();

  self->resolver = ResolveSymbol_create();
}

/**Function********************************************************************

  Synopsis           [Private destructor used by clas destroyer]

  Description        []

  SideEffects        []

  SeeAlso            [symb_table_deinit]

******************************************************************************/
static assoc_retval class_layers_hash_free(char *key, char *data, char * arg)
{
  array_t* _class = (array_t*) data;

  if (_class != NULL) {
    const char* name; int i;
    arrayForEachItem(const char*, _class, i, name) { FREE(name); }
    array_free(_class);
  }
  return ASSOC_DELETE;
}


/**Function********************************************************************

  Synopsis           [Private deinitializer]

  Description        [Private deinitializer, called by the destructor only]

  SideEffects        []

  SeeAlso            [symb_table_init]

******************************************************************************/
static void symb_table_deinit(SymbTable_ptr self)
{
  ListIter_ptr iter;

  free_assoc(self->expr_simplify_hash);
  self->expr_simplify_hash = NULL;

  TypeChecker_destroy(self->tc);
  self->tc = TYPE_CHECKER(NULL);

  /* destroys the layer classes: */
  free_list(self->class_names);
  clear_assoc_and_free_entries(self->class_layers, class_layers_hash_free);
  free_assoc(self->class_layers);

  if (self->default_class_name != (const char*) NULL) {
    FREE(self->default_class_name);
  }

  /* get rid of all contained layers */
  iter = NodeList_get_first_iter(self->layers);
  while (! ListIter_is_end(iter)) {
    /* The cache will be destroyed, no need for cleaning it */
    SymbLayer_destroy_raw(SYMB_LAYER(NodeList_get_elem_at(self->layers, iter)));
    iter = ListIter_get_next(iter);
  }
  NodeList_destroy(self->layers);

  SymbCache_destroy(self->cache);
  free_assoc(self->categories);

  free_assoc(self->name2layer);

  ResolveSymbol_destroy(self->resolver);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given name corresponds to an already
  registered layer.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean
symb_table_layer_exists(const SymbTable_ptr self, const char* layer_name)
{
  return (SymbTable_get_layer(self, layer_name) != SYMB_LAYER(NULL));
}

/**Function********************************************************************

  Synopsis [Given a list of symbols and a list of layers names,
  returns a new list that contains only those symbols that have been
  declared within the given layers]

  Description [The caller is responsible for destroying the
  returned list]

  SideEffects        []

  See Also           []

******************************************************************************/
static NodeList_ptr
symb_table_filter_layers_symbols(SymbTable_ptr self,
                                 const array_t* layer_names,
                                 SymbTableIter* iter)
{
  NodeList_ptr res;
  const char* layer_name;
  int idx;

  res = NodeList_create();

  while (!SymbTable_iter_is_end(self, iter)) {
    node_ptr sym = SymbTable_iter_get_symbol(self, iter);

    arrayForEachItem(const char*, (array_t*) layer_names, idx, layer_name) {
      SymbLayer_ptr layer = SymbTable_get_layer(self, layer_name);

      if (layer != SYMB_LAYER(NULL)) {
        if (SymbLayer_is_symbol_in_layer(layer, sym) &&
            !NodeList_belongs_to(res, sym)) {
          NodeList_append(res, sym);
        }
      }
    }

    SymbTable_iter_next(self, iter);
  }

  return res;
}


/**Function********************************************************************

  Synopsis [Removes or rename a given layer inside the given
  layers class. If class_name is NULL, then default class is taken
  (must be set before)]

  Description        [Used internally by remove_layer and rename_layer.
  To remove, pass NULL as new_layer_name]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void symb_table_layer_rename_in_class(SymbTable_ptr self,
                                             const char* class_name,
                                             const char* old_layer_name,
                                             const char* new_layer_name)
{
  array_t* _class;
  const char* name; int i;

  SYMB_TABLE_CHECK_INSTANCE(self);

  _class = SymbTable_get_class_layer_names(self, class_name);
  arrayForEachItem(const char*, _class, i, name) {
    if (strcmp(name, old_layer_name) == 0) {
      FREE(name);
      if (new_layer_name != (const char*) NULL) {
        array_insert(const char*, _class, i,
                     util_strsav((char*) new_layer_name));
      }
      else {
        /* removing, shifts to fill the hole */
        int j;
        for (j=i+1; j < array_n(_class); ++j) {
          array_insert(const char*, _class, j-1,
                       array_fetch(const char*, _class, j));
        }
        _class->num -= 1;
      }

      break;
    }
  }
}


/**Function********************************************************************

  Synopsis [Internal service used by methods that handle layer classes]

  Description        [Returns the array of layer class, or NULL when not existing.
  Resolves NULL class_name to the default class]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static array_t* symb_table_get_layers_from_class(const SymbTable_ptr self,
                                                 const char* class_name)
{
  string_ptr key;
  if (class_name == (const char*) NULL) {
    class_name = SymbTable_get_default_layers_class_name(self);
  }
  nusmv_assert(class_name != (const char*) NULL); /* must be set before */

  key = find_string((char*) class_name);
  return (array_t*) find_assoc(self->class_layers, (node_ptr) key);
}


/**Function********************************************************************

  Synopsis [Internal service used by methods that handle layer classes]

  Description        [Returns the array of layer class, creating it when
  needed]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static array_t* symb_table_create_layers_class(const SymbTable_ptr self,
                                               const char* class_name)
{
  string_ptr key;
  array_t* _class;

  if (class_name == (const char*) NULL) {
    class_name = SymbTable_get_default_layers_class_name(self);
  }
  nusmv_assert(class_name != (const char*) NULL); /* must be set before */

  key = find_string((char*) class_name);
  _class = (array_t*) find_assoc(self->class_layers, (node_ptr) key);

  if (_class == (array_t*) NULL) {
    _class = array_alloc(const char*, 1);
    insert_assoc(self->class_layers, (node_ptr) key, (node_ptr) _class);
    self->class_names = cons((node_ptr) get_text(key), self->class_names);
  }

  return _class;
}


/**Function********************************************************************

  Synopsis [Internal service used by methods that handle layer classes]

  Description        [Returns the array of layer class, NULL if class does
  not exist.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static array_t* symb_table_get_layers_class(const SymbTable_ptr self,
                                            const char* class_name)
{
  string_ptr key;
  array_t* _class;

  if (class_name == (const char*) NULL) {
    class_name = SymbTable_get_default_layers_class_name(self);
  }
  nusmv_assert(class_name != (const char*) NULL); /* must be set before */

  key = find_string((char*) class_name);
  _class = (array_t*) find_assoc(self->class_layers, (node_ptr) key);

  return _class;
}


/**Function********************************************************************

  Synopsis           [Internal service used by
                      SymbTable_get_array_define_flatten_body]

  Description        [Returns the flattened body of an array define]

  SideEffects        []

  SeeAlso            [SymbTable_get_array_define_flatten_body]

******************************************************************************/
static node_ptr
symb_table_flatten_array_define(const SymbTable_ptr self,
                                const node_ptr body,
                                const node_ptr context)
{
  if (Nil == body) {
    return Nil;
  }

  switch (node_get_type(body)) {
  case ARRAY_DEF:
  case CONS:
    return find_node(node_get_type(body),
                     symb_table_flatten_array_define(self, car(body), context),
                     symb_table_flatten_array_define(self, cdr(body), context));

  default:
    return Compile_FlattenSexp(self, body, context);
  }
}

/**Function********************************************************************

  Synopsis    [Returns the type of a define.]

  Description [Returns the type of a define, for a discussion on
               symbol categories look the SymbCategory enum
               description in SymbTable.h]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static SymbCategory symb_table_detect_expr_category(const SymbTable_ptr st,
                                                    const Expr_ptr expr)
{
  /* but return a bitwise OR of VFT_... values instead of list of vars.
     Thus the implementation has to be very similar as well.
     Otherwise, it is likely that there are bugs here now */

  SymbCategory cat = SYMBOL_INVALID;
  int ta;

  nusmv_assert(Nil != expr);

  ta = node_get_type(expr);

  /* We are not going to memoize results for constants and symbols,
     because we may infeer with the internal category memoization,
     which associates symbols with the respective category. We only
     memoize non-atomic expressions: in this way we do not conflict
     with the previous memoization */
  if (NEXT == node_get_type(expr)) {
    cat = SYMBOL_NEXT_DEFINE;
  }
  else if (node_is_leaf(expr)) {
    cat = SYMBOL_CONSTANT;
  }
  else if (DOT == ta || ATOM == ta || ARRAY == ta) {

    if (SymbTable_is_symbol_constant(st, expr)) {
      cat = SYMBOL_CONSTANT;
    }
    else if (SymbTable_is_symbol_state_var(st, expr) ||
             SymbTable_is_symbol_frozen_var(st, expr)) {
      cat = SYMBOL_STATE_DEFINE;
    }
    else if (SymbTable_is_symbol_input_var(st, expr)) {
      cat = SYMBOL_INPUT_DEFINE;
    }
  }
  else {
    /* Defines / params.. should be already given expanded. */
    nusmv_assert(!SymbTable_is_symbol_declared(st, expr));
    SymbCategory l_cat = SYMBOL_INVALID;
    SymbCategory r_cat = SYMBOL_INVALID;

    boolean has_next;
    boolean has_state;
    boolean has_input;

    /* Check for memoized data */
    cat = NODE_TO_INT(find_assoc(st->categories, expr));
    if (SYMBOL_INVALID != cat) {
      return cat;
    }

    /* No memoized data found.. proceed with expression analysis */
    if (Nil != car(expr)) {
      l_cat = symb_table_detect_expr_category(st, car(expr));
    }

    if (Nil != cdr(expr)) {
      r_cat = symb_table_detect_expr_category(st, cdr(expr));
    }

    has_next = ((SYMBOL_STATE_INPUT_NEXT_DEFINE == l_cat) ||
                (SYMBOL_STATE_INPUT_NEXT_DEFINE == r_cat) ||
                (SYMBOL_INPUT_NEXT_DEFINE == l_cat) ||
                (SYMBOL_INPUT_NEXT_DEFINE == r_cat) ||
                (SYMBOL_STATE_NEXT_DEFINE == l_cat) ||
                (SYMBOL_STATE_NEXT_DEFINE == r_cat) ||
                (SYMBOL_NEXT_DEFINE == l_cat) ||
                (SYMBOL_NEXT_DEFINE == r_cat));

    has_input = ((SYMBOL_STATE_INPUT_NEXT_DEFINE == l_cat) ||
                 (SYMBOL_STATE_INPUT_NEXT_DEFINE == r_cat) ||
                 (SYMBOL_INPUT_NEXT_DEFINE == l_cat) ||
                 (SYMBOL_INPUT_NEXT_DEFINE == r_cat) ||
                 (SYMBOL_STATE_INPUT_DEFINE == l_cat) ||
                 (SYMBOL_STATE_INPUT_DEFINE == r_cat) ||
                 (SYMBOL_INPUT_DEFINE == l_cat) ||
                 (SYMBOL_INPUT_DEFINE == r_cat));

    has_state = ((SYMBOL_STATE_INPUT_NEXT_DEFINE == l_cat) ||
                 (SYMBOL_STATE_INPUT_NEXT_DEFINE == r_cat) ||
                 (SYMBOL_STATE_NEXT_DEFINE == l_cat) ||
                 (SYMBOL_STATE_NEXT_DEFINE == r_cat) ||
                 (SYMBOL_STATE_INPUT_DEFINE == l_cat) ||
                 (SYMBOL_STATE_INPUT_DEFINE == r_cat) ||
                 (SYMBOL_STATE_DEFINE == l_cat) ||
                 (SYMBOL_STATE_DEFINE == r_cat));

    if (has_next && has_input && has_state) {
      cat = SYMBOL_STATE_INPUT_NEXT_DEFINE;
    }
    else if (has_next && has_input) {
      cat = SYMBOL_INPUT_NEXT_DEFINE;
    }
    else if (has_next && has_state) {
      cat = SYMBOL_STATE_NEXT_DEFINE;
    }
    else if (has_state && has_input) {
      cat = SYMBOL_STATE_INPUT_DEFINE;
    }
    else if (has_input) {
      cat = SYMBOL_INPUT_DEFINE;
    }
    else if (has_next) {
      cat = SYMBOL_NEXT_DEFINE;
    }
    else if (has_state) {
      cat = SYMBOL_STATE_DEFINE;
    }
    else if ((SYMBOL_CONSTANT == l_cat) ||
             (SYMBOL_CONSTANT == r_cat)) {
      cat = SYMBOL_CONSTANT;
    }
    else {
      /* the only remaining values */
      nusmv_assert(SYMBOL_INVALID == l_cat && SYMBOL_INVALID == r_cat);

      cat = SYMBOL_INVALID;
    }

    /* Memoize the result for the given expression. */
    insert_assoc(st->categories, expr, NODE_FROM_INT(cat));
  }

  return cat;
}
