/**CFile***********************************************************************

  FileName    [BaseEnc.c]

  PackageName [enc.base]

  Synopsis    [Implementaion of pure base class 'BaseEnc']

  Description []

  SeeAlso     [BaseEnc.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.base'' package of NuSMV version 2.
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

  Revision    [$Id: BaseEnc.c,v 1.1.2.10.6.6 2009-09-17 11:49:49 nusmv Exp $]

******************************************************************************/

#include "BaseEnc.h"
#include "BaseEnc_private.h"

#include "utils/utils.h"

static char rcsid[] UTIL_UNUSED = "$Id: BaseEnc.c,v 1.1.2.10.6.6 2009-09-17 11:49:49 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'BaseEnc_private.h' for class 'BaseEnc' definition. */

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

static void base_enc_finalize ARGS((Object_ptr object, void* dummy));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [The BaseEnc class destructor]

  Description        [The BaseEnc class destructor. Since this class is pure
  there is no constructor.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
VIRTUAL void BaseEnc_destroy(BaseEnc_ptr self)
{
  BASE_ENC_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis           [Returns true whether the given layer name is the
  name of a layer that is currently committed to self.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean BaseEnc_layer_occurs(const BaseEnc_ptr self, const char* layer_name)
{
  SymbLayer_ptr layer;

  BASE_ENC_CHECK_INSTANCE(self);

  layer = SymbTable_get_layer(self->symb_table, layer_name);
  return NodeList_belongs_to(self->committed_layers, (node_ptr) layer);
}


/**Function********************************************************************

  Synopsis           [Returns the list of the committed layers]

  Description [Returned list is a list of SymbLayer instances. The
  returned list is ordered wrt to layers insert policy. The list and
  its content still belongs to self, do not destroy or change it]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr BaseEnc_get_committed_layers(const BaseEnc_ptr self)
{
  BASE_ENC_CHECK_INSTANCE(self);
  return self->committed_layers;
}


/**Function********************************************************************

  Synopsis           [Returns the list of names of the committed layers]

  Description [Returned array belongs to self. Do not store it
  permanently, change or delete it. If you commit or remove a
  layer into self, any previoulsy returned array will become
  invalid.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
const array_t* BaseEnc_get_committed_layer_names(BaseEnc_ptr self)
{
  BASE_ENC_CHECK_INSTANCE(self);

  /* allocates the list if needed */
  if (self->layer_names == (array_t*) NULL) {
    ListIter_ptr iter;

    self->layer_names = array_alloc(const char*,
                   NodeList_get_length(self->committed_layers));
    nusmv_assert(self->layer_names != (array_t*) NULL);

    for (iter = NodeList_get_first_iter(self->committed_layers);
         !ListIter_is_end(iter);
         iter = ListIter_get_next(iter)) {
      SymbLayer_ptr layer =
        SYMB_LAYER(NodeList_get_elem_at(self->committed_layers, iter));
      array_insert_last(const char*, (self->layer_names),
                        SymbLayer_get_name(layer));
    }
  }

  return self->layer_names;
}


/**Function********************************************************************

  Synopsis           [Returns the SymbTable that self uses]

  Description [Returns the SymbTable that self uses. Returned instance
  belongs to self, do not destroy it.]

  SideEffects        []

  SeeAlso            [BaseEnc_get_type_checker]

******************************************************************************/
SymbTable_ptr BaseEnc_get_symb_table(const BaseEnc_ptr self)
{
  BASE_ENC_CHECK_INSTANCE(self);
  return self->symb_table;
}


/**Function********************************************************************

  Synopsis           [Returns the type checker instance owned by the
                      SymbTable that self uses]

  Description        [Returns the type checker instance owned by the
                      SymbTable that self uses. Returned instance
                      belongs to self, do not destroy it.]

  SideEffects        []

  SeeAlso            [BaseEnc_get_symb_table]

******************************************************************************/
TypeChecker_ptr BaseEnc_get_type_checker(const BaseEnc_ptr self)
{
  BASE_ENC_CHECK_INSTANCE(self);
  return SymbTable_get_type_checker(self->symb_table);
}


/**Function********************************************************************

  Synopsis           [Call this method to enter a new layer. All variables
  occurring in the layer, will be encoded as a result.]

  Description        [This method is virtual. The result of the encoding
  depends on the actual instance (its actual class) it is invoked on.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
VIRTUAL void
BaseEnc_commit_layer(BaseEnc_ptr self, const char* layer_name)
{
  BASE_ENC_CHECK_INSTANCE(self);
  self->commit_layer(self, layer_name);
}


/**Function********************************************************************

  Synopsis           [Call this method to remove an already committed layer.
  All variables occurring in the layer will be removed as a result. It will no
  longer allowed to use those variables within expressions encoded by self]

  Description        [This method is virtual. The result of the removal
  depends on the actual instance (its actual class) it is invoked on.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
VIRTUAL void
BaseEnc_remove_layer(BaseEnc_ptr self, const char* layer_name)
{
  BASE_ENC_CHECK_INSTANCE(self);
  self->remove_layer(self, layer_name);
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BaseEnc class private initializer]

  Description        [The BaseEnc class private initializer]

  SideEffects        []

  SeeAlso            [BaseEnc_create]

******************************************************************************/
void base_enc_init(BaseEnc_ptr self, SymbTable_ptr symb_table)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */
  self->symb_table = symb_table;
  self->committed_layers = NodeList_create();
  self->layer_names = (array_t*) NULL;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = base_enc_finalize;

  /* these must be inherited, and called by inherited methods: */
  OVERRIDE(BaseEnc, commit_layer) = base_enc_commit_layer;
  OVERRIDE(BaseEnc, remove_layer) = base_enc_remove_layer;
}


/**Function********************************************************************

  Synopsis           [The BaseEnc class private deinitializer]

  Description        [The BaseEnc class private deinitializer]

  SideEffects        []

  SeeAlso            [BaseEnc_destroy]

******************************************************************************/
void base_enc_deinit(BaseEnc_ptr self)
{
  ListIter_ptr iter;

  /* members deinitialization */

  /* unlock all layers that are still committed in */
  iter = NodeList_get_first_iter(self->committed_layers);
  while (! ListIter_is_end(iter)) {
    SymbLayer_ptr lyr;
    lyr = SYMB_LAYER(NodeList_get_elem_at(self->committed_layers, iter));
    SymbLayer_removed_from_enc(lyr);
    iter = ListIter_get_next(iter);
  }
  NodeList_destroy(self->committed_layers);

  if (self->layer_names != (array_t*) NULL) {
    array_free(self->layer_names);
    self->layer_names = (array_t*) NULL;
  }

  /* base class deinitialization */
  object_deinit(OBJECT(self));
}


/**Function********************************************************************

  Synopsis           [Register and store a new layer to be committed]

  Description        [This method must always be called by derived classes]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void base_enc_commit_layer(BaseEnc_ptr self, const char* layer_name)
{
  SymbLayer_ptr layer;

  /* not already added: */
  nusmv_assert(!BaseEnc_layer_occurs(self, layer_name));

  layer = SymbTable_get_layer(self->symb_table, layer_name);

  /* register as user to the layer, and add the layer at the right level */
  SymbLayer_committed_to_enc(layer);

  {
    ListIter_ptr iter = NodeList_get_first_iter(self->committed_layers);
    while (!ListIter_is_end(iter)) {
      if (SymbLayer_must_insert_before(layer,
             SYMB_LAYER(NodeList_get_elem_at(self->committed_layers, iter)))) {
        NodeList_insert_before(self->committed_layers, iter, (node_ptr) layer);
        break;
      }

      iter = ListIter_get_next(iter);
    }

    if (ListIter_is_end(iter)) {
      /* reached the end and not inserted: append */
      NodeList_append(self->committed_layers, (node_ptr) layer);
    }
  }

  /* frees the layer names (will be possibly re-calculated) */
  if (self->layer_names != (array_t*) NULL) {
    array_free(self->layer_names);
    self->layer_names = (array_t*) NULL;
  }
}


/**Function********************************************************************

  Synopsis           [Unregister and remove from the list of layers
  the given layer.]

  Description [This method must always be called by derived methods,
  after they have done their work.

  WARNING: If the layer has been
  renamed after having been committed, it is the *new* name (the name
  the layer has when it is being removed) that must be used, and *not*
  the name that had been used when commiting it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void base_enc_remove_layer(BaseEnc_ptr self, const char* layer_name)
{
  SymbLayer_ptr layer;
  ListIter_ptr iter;

  /* must be belonging to the layers list: */
  nusmv_assert(BaseEnc_layer_occurs(self, layer_name));

  layer = SymbTable_get_layer(self->symb_table, layer_name);

  /* search it, then unregister and remove it */
  iter = NodeList_get_first_iter(self->committed_layers);
  while (!ListIter_is_end(iter)) {
    if (layer == SYMB_LAYER(NodeList_get_elem_at(self->committed_layers,
                                                 iter))) {
      NodeList_remove_elem_at(self->committed_layers, iter);
      SymbLayer_removed_from_enc(layer);
      break;
    }
    iter = ListIter_get_next(iter);
  }

  /* frees the layer names (will be possibly re-calculated) */
  if (self->layer_names != (array_t*) NULL) {
    array_free(self->layer_names);
    self->layer_names = (array_t*) NULL;
  }

}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The BaseEnc class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void base_enc_finalize(Object_ptr object, void* dummy)
{
  BaseEnc_ptr self = BASE_ENC(object);

  base_enc_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/

