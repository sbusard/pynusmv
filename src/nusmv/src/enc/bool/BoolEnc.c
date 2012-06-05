/**CFile***********************************************************************

  FileName    [BoolEnc.c]

  PackageName [enc.bool]

  Synopsis    [Implementaion of class 'BoolEnc']

  Description []

  SeeAlso     [BoolEnc.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bool'' package of NuSMV version 2.
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

  Revision    [$Id: BoolEnc.c,v 1.1.2.22.4.37 2010-01-30 15:17:33 nusmv Exp $]

******************************************************************************/

#include "BoolEnc.h"
#include "BoolEnc_private.h"

#include "enc/encInt.h"
#include "compile/compile.h"
#include "parser/symbols.h"
#include "set/set.h"

#include "utils/WordNumber.h"
#include "utils/utils.h"
#include "utils/error.h"

#include "utils/ustring.h"


static char rcsid[] UTIL_UNUSED = "$Id: BoolEnc.c,v 1.1.2.22.4.37 2010-01-30 15:17:33 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define BOOL_ENC_DEFAULT_LAYER_SUFFIX \
  "_bool"

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'BoolEnc_private.h' for class 'BoolEnc' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

static node_ptr boolean_type = (node_ptr) NULL;

/**Variable********************************************************************

  Synopsis     [A global variable shared among all the BoolEnc class
  instances]

  Description  [Initialized the first time an instance of class BoolEnc is
  created (see SymbTable constructor), and deinitialized when the last one
  is destroyed]

******************************************************************************/
static int bool_enc_instances = 0;


/**Variable********************************************************************

  Synopsis     [A global variable shared among all the BoolEnc class
  instances]

  Description [Contains information about all layers that are
  created and used by any BoolEnc. This is used to handle
  deallocation of layers when the last owning encoder is gone. The
  hash associates layer instances with a counter representing the
  number of users of that layer. When the last user is being
  destroyed, or when the layer is begin removed from the last
  user, the layer is removed from the symbol table which it is
  contained into. When the number of bool enc instances goes to 0,
  the hash is freed.]

******************************************************************************/
static hash_ptr bool_enc_owned_layers = (hash_ptr) NULL;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/* warning: the first of these set to 1 will be taken (see issue 2549) */
#define BOOL_ENCODING_HIGHER_TO_LOWER_BALANCED  1 /* this is best in average */

/* these are kept for record and testing */
#define BOOL_ENCODING_LOWER_TO_HIGHER_BALANCED  0 /* the old default */
#define BOOL_ENCODING_HIGHER_TO_LOWER_INCREMENTAL  0 /* this is better
                                                        in some cases */

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void bool_enc_finalize ARGS((Object_ptr object, void* dummy));

static void
bool_enc_encode_var ARGS((BoolEnc_ptr self, node_ptr var,
                          SymbLayer_ptr src_layer, SymbLayer_ptr dest_layer));

static node_ptr
bool_enc_encode_scalar_var ARGS((BoolEnc_ptr self, node_ptr name, int suffix,
                                 node_ptr values,
                                 SymbLayer_ptr src_layer,
                                 SymbLayer_ptr dest_layer));

static void
bool_enc_set_var_encoding ARGS((BoolEnc_ptr self, node_ptr name,
                                node_ptr enc));

static node_ptr
bool_enc_get_var_encoding ARGS((const BoolEnc_ptr self, node_ptr name));

static void
bool_enc_traverse_encoding ARGS((const BoolEnc_ptr self,
                                 node_ptr enc, NodeList_ptr list));

static node_ptr
bool_enc_get_var_mask_recur ARGS((const BoolEnc_ptr self,
                                  node_ptr enc,
                                  NodeList_ptr cube,
                                  ListIter_ptr cube_iter));

static node_ptr
bool_enc_compute_set_encoding ARGS((BoolEnc_ptr self, node_ptr set,
                                    node_ptr bit_prefix, int bit_suffix,
                                    Set_t* out_bits, boolean top));

static boolean bool_enc_is_boolean_range ARGS((node_ptr values));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BoolEnc class constructor]

  Description        [The BoolEnc class constructor]

  SideEffects        []

  SeeAlso            [BoolEnc_destroy]

******************************************************************************/
BoolEnc_ptr BoolEnc_create(SymbTable_ptr symb_table)
{
  BoolEnc_ptr self = ALLOC(BoolEnc, 1);
  BOOL_ENC_CHECK_INSTANCE(self);

  bool_enc_init(self, symb_table);
  return self;
}


/**Function********************************************************************

  Synopsis           [The BoolEnc class destructor]

  Description        [The BoolEnc class destructor]

  SideEffects        []

  SeeAlso            [BoolEnc_create]

******************************************************************************/
VIRTUAL void BoolEnc_destroy(BoolEnc_ptr self)
{
  BOOL_ENC_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given symbol is the name of
  a bit variable that is part of a scalar var]

  Description        []

  SideEffects        []

  SeeAlso            [BoolEnc_get_scalar_var_of_bit]

******************************************************************************/
boolean BoolEnc_is_var_bit(const BoolEnc_ptr self, node_ptr name)
{
  BOOL_ENC_CHECK_INSTANCE(self);
  return (node_get_type(name) == BIT);
}


/**Function********************************************************************

  Synopsis           [Returns true if the given symbol is the name of
  a scalar (non-boolean) variable]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean BoolEnc_is_var_scalar(const BoolEnc_ptr self, node_ptr name)
{
  node_ptr enc;

  BOOL_ENC_CHECK_INSTANCE(self);

  enc = BoolEnc_get_var_encoding(self, name);
  return enc != boolean_type;
}


/**Function********************************************************************

  Synopsis           [Returns the name of the scalar variable whose
  the given bit belongs]

  Description        [Returns the name of the scalar variable whose
  the given bit belongs. The given var MUST be a bit]

  SideEffects        []

  SeeAlso            [BoolEnc_is_var_bit, BoolEnc_get_index_from_bit]

******************************************************************************/
node_ptr BoolEnc_get_scalar_var_from_bit(const BoolEnc_ptr self, node_ptr name)
{
  BOOL_ENC_CHECK_INSTANCE(self);
  nusmv_assert(BoolEnc_is_var_bit(self, name));

  return car(name);
}


/**Function********************************************************************

  Synopsis           [Given a scalar variable name, construct the name for
                      the nth-indexed bit.]

  Description        [Constructs and returns the name of the nth-indexed bit
                      of the given scalar variable]

  SideEffects        []

  SeeAlso            [BoolEnc_is_var_bit, BoolEnc_get_index_from_bit]

******************************************************************************/
node_ptr BoolEnc_make_var_bit(const BoolEnc_ptr self, node_ptr name, int index)
{
  BOOL_ENC_CHECK_INSTANCE(self);
  return find_node(BIT, name, NODE_FROM_INT(index));
}


/**Function********************************************************************

  Synopsis           [Returns the index of given bit.]

  Description        [The given var MUST be a bit]

  SideEffects        []

  SeeAlso            [BoolEnc_is_var_bit, BoolEnc_get_scalar_var_from_bit]

******************************************************************************/
int BoolEnc_get_index_from_bit(const BoolEnc_ptr self, node_ptr name)
{
  BOOL_ENC_CHECK_INSTANCE(self);
  nusmv_assert(BoolEnc_is_var_bit(self, name));
  return NODE_TO_INT(cdr(name));
}



/**Function********************************************************************

  Synopsis [Returns the list of boolean vars used in the encoding of
  given scalar var]

  Description        [Returned list must be destroyed by the caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr BoolEnc_get_var_bits(const BoolEnc_ptr self, node_ptr name)
{
  NodeList_ptr res;
  node_ptr enc;

  BOOL_ENC_CHECK_INSTANCE(self);

  enc = BoolEnc_get_var_encoding(self, name);
  res = NodeList_create();
  bool_enc_traverse_encoding(self, enc, res);
  return res;
}


/**Function********************************************************************

  Synopsis           [Given a variable, returns its boolean encoding]

  Description        [Given variable must have been encoded by self]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr BoolEnc_get_var_encoding(const BoolEnc_ptr self, node_ptr name)
{
  node_ptr enc;

  BOOL_ENC_CHECK_INSTANCE(self);

  enc = bool_enc_get_var_encoding(self, name);
  nusmv_assert(enc != Nil); /* must be previoulsy encoded */
  return enc;
}


/**Function********************************************************************

Synopsis           [Given a set of constants values (for example, the domain
                    of a scalar variable), calculates its boolean
                    encoding by introducing boolean symbols that
                    are returned along with the resulting
                    encoding.]

Description        [This method can be used to retrieve the boolean
                    encoding of a given set of symbols.

                    For example, it may be used to calculate the
                    boolean encoding representing the domain of a
                    scalar variable which has not been added to any
                    layer. It returns the boolean encoding
                    (typically a ITE node) and the set of boolean
                    symbols (bits) that have been introduced in the
                    encoding. Important: the introduced boolean
                    symbols are not variables, as they are not
                    declared into the symbol table. It is up to the
                    caller later to declare them if needed.

                    The introduced symbol names are guaranteed to
                    be not among the currently declared symbols.

                    To retrieve the boolean encoding of an existing
                    (and committed) variable, use method
                    get_var_encoding instead.]

SideEffects        [Passed set is filled with symbol bits occurring in the
                     encoding. No memoization or change is performed.]

SeeAlso            [BoolEnc_get_var_encoding]

******************************************************************************/
node_ptr BoolEnc_get_values_bool_encoding(const BoolEnc_ptr self,
                                          node_ptr values,
                                          Set_t* bits)
{
  node_ptr var_name;

  BOOL_ENC_CHECK_INSTANCE(self);

  var_name = SymbTable_get_determinization_var_name(BASE_ENC(self)->symb_table);
  return bool_enc_compute_set_encoding(self, values, var_name, 0, bits, true);
}


/**Function********************************************************************

  Synopsis           [Given a variable, it returns the mask of its encoding]

  Description        [Returns an expression representing the mask that
  removes repetitions of leaves in a variable encoding by assigning
  value false to don't care boolean variables.Forr Boolean variables
  it returns the expression TRUE. Similarly for Word variables (since
  for words there are non redundant assignments).

  As an example of what this function does, let us consider a variable
  x having range 0..4. It can be encoded with 3 bits are needed to
  encode it: x0, x1, x2. The encodeding performed by NuSMV is

     ITE(x0, ITE(x1, 1, 3), ITE(x1, 2, ITE(x2, 4,  0))).

  Thus x=2 corresponds to assignment !x0&x1 where x2 is a dont'care.
  Similarly for x=1 and x=3 (for x=0 and x=4) there is a unique
  complete assignment to the x0, x1, x2 variables that represent the
  respective encoding). This function fixes a value for x2 in the
  assignments representing x=2, x=1 and x=3 respectively (it force x2
  to be false). Thus it builds the formula in this case:

     ITE(x0, ITE(x2, 0, 1), ITE(x1, 1, ITE(x2, 0,  1)))

  that removes the redundant assignments where needed.

  Result is memoized. See issue 0925 for further details.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr BoolEnc_get_var_mask(const BoolEnc_ptr self, node_ptr name)
{
  node_ptr enc;
  NodeList_ptr cube;
  ListIter_ptr iter;
  node_ptr res;
  SymbType_ptr var_type;
  SymbTable_ptr st;

  BOOL_ENC_CHECK_INSTANCE(self);

  st = BaseEnc_get_symb_table(BASE_ENC(self));
  var_type = SymbTable_get_var_type(st, name);

  /* Computing the mask is meaningful only for enumerative non
     boolean variables: */
  if (SymbType_is_enum(var_type) && !SymbType_is_boolean(var_type)) {
    /* check memoized mask */
    res = find_assoc(self->var2mask, name);
    if (Nil != res) return res; /* hit */

    enc = BoolEnc_get_var_encoding(self, name);
    nusmv_assert(Nil != enc); /* must be previoulsy encoded */
    cube = BoolEnc_get_var_bits(self, name);

    NODE_LIST_CHECK_INSTANCE(cube);

    iter = NodeList_get_first_iter(cube);

    res = bool_enc_get_var_mask_recur(self, enc, cube, iter);

    NodeList_destroy(cube);

    /* memoizes the result */
    insert_assoc(self->var2mask, name, res);
  }
  else {
    res = Expr_true();
  }

  return res;
}


/**Function********************************************************************

  Synopsis [Given a BitValues instance already set with an
  assigments for its bits, returns the corresponding value for the
  scalar or word variable whose bits are onctained into the
  BitValues instance]

  Description [Returns an ATOM, a NUMBER, an NUMBER_UNSIGNED_WORD,
  etc. depending on the kind of variable. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr BoolEnc_get_value_from_var_bits(const BoolEnc_ptr self,
                                         const BitValues_ptr bit_values)
{
  node_ptr var = BitValues_get_scalar_var(bit_values);
  SymbType_ptr var_type = SymbTable_get_var_type(
                           BaseEnc_get_symb_table(BASE_ENC(self)), var);
  node_ptr enc = BoolEnc_get_var_encoding(self, var);

  if (SymbType_is_enum(var_type)) {
    while (true) {
      if (enc == Nil) return Nil;

      switch (node_get_type(enc)) {
      case IFTHENELSE: {
        int bit_index = BoolEnc_get_index_from_bit(self, car(car(enc)));
        switch (BitValues_get(bit_values, bit_index)) {
        case BIT_VALUE_FALSE: enc = cdr(enc); break;
        case BIT_VALUE_TRUE:
        case BIT_VALUE_DONTCARE: enc = cdr(car(enc)); break;
        default: error_unreachable_code(); /* no other known values */
        }
        break;
      }

      case DOT: case ATOM: case NUMBER: case TRUEEXP: case FALSEEXP:
        return enc; /* found the value */

      default: error_unreachable_code(); /* no other known cases */
      }
    } /* traversing loop */
  } /* a scalar variable */

  if (SymbType_is_word(var_type)) {
    /* re-constructs word number value from values of bits */
    WordNumberValue value;
    node_ptr iter;
    nusmv_assert(node_get_type(enc) == UNSIGNED_WORD ||
                 node_get_type(enc) == SIGNED_WORD);

    value = 0ULL;
    for (iter = car(enc); iter != Nil; iter = cdr(iter)) {
      node_ptr bit;
      int bit_index;
      nusmv_assert(node_get_type(iter) == CONS);
      bit = car(iter);
      bit_index = BoolEnc_get_index_from_bit(self, bit);

      value = value << 1;
      switch (BitValues_get(bit_values, bit_index)) {
      case BIT_VALUE_FALSE: break;
      case BIT_VALUE_TRUE:
      case BIT_VALUE_DONTCARE: value |= 1ULL; break;
      default: error_unreachable_code(); /* no other known values */
      }

    }

    {
      WordNumber_ptr wn;
      wn = WordNumber_from_integer(value, SymbType_get_word_width(var_type));
      /* representation of vars is always UNSIGNED (as on 2010.01.30).
         but constants can be signed and unsigned.
         check the type for var for sign instead */
      if (SymbType_is_unsigned_word(var_type)) {
        return find_node(NUMBER_UNSIGNED_WORD, (node_ptr) wn, Nil);
      }
      else return find_node(NUMBER_SIGNED_WORD, (node_ptr) wn, Nil);
    }
  } /* a word variable */

  error_unreachable_code(); /* no other known var types */
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BoolEnc class private initializer]

  Description        [The BoolEnc class private initializer]

  SideEffects        []

  SeeAlso            [BoolEnc_create]

******************************************************************************/
void bool_enc_init(BoolEnc_ptr self, SymbTable_ptr symb_table)
{
  /* base class initialization */
  base_enc_init(BASE_ENC(self), symb_table);

  /* static members */
  /* init a shared variable (done by the first constructor call only) */
  if (boolean_type == (node_ptr) NULL) {
    nusmv_assert(bool_enc_instances == 0);
    boolean_type = find_node(BOOLEAN, Nil, Nil);
  }

  if (bool_enc_owned_layers == (hash_ptr) NULL) {
    nusmv_assert(bool_enc_instances == 0);
    bool_enc_owned_layers = new_assoc();
    nusmv_assert(bool_enc_owned_layers != (hash_ptr) NULL);
  }

  bool_enc_instances += 1;

  /* members initialization */
  self->var2enc = new_assoc();
  self->var2mask = new_assoc();

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = bool_enc_finalize;

  /* inherited by BaseEnc: */
  OVERRIDE(BaseEnc, commit_layer) = bool_enc_commit_layer;
  OVERRIDE(BaseEnc, remove_layer) = bool_enc_remove_layer;
}


/**Function********************************************************************

  Synopsis           [The BoolEnc class private deinitializer]

  Description        [The BoolEnc class private deinitializer]

  SideEffects        []

  SeeAlso            [BoolEnc_destroy]

******************************************************************************/
void bool_enc_deinit(BoolEnc_ptr self)
{
  /* members deinitialization */
  free_assoc(self->var2mask);
  free_assoc(self->var2enc);

  { /* destroys all owned layers if self is the last user: */
    NodeList_ptr layers = BaseEnc_get_committed_layers(BASE_ENC(self));
    ListIter_ptr iter = NodeList_get_first_iter(layers);
    while (!ListIter_is_end(iter)) {
      SymbLayer_ptr lyr;
      int count;
      lyr = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
      count = PTR_TO_INT(find_assoc(bool_enc_owned_layers, (node_ptr) lyr));
      nusmv_assert(count >= 0);

      if (count == 1) { /* the layer has to be destroyed */
        ListIter_ptr niter = ListIter_get_next(iter);

        SymbLayer_removed_from_enc(lyr);

        /* SymbTable_remove_layer also performs the destruction of lyr */
        SymbTable_remove_layer(BASE_ENC(self)->symb_table, lyr);

        insert_assoc(bool_enc_owned_layers, (node_ptr) lyr,
                     PTR_FROM_INT(node_ptr, 0));
        NodeList_remove_elem_at(layers, iter);

        iter = niter;
        continue;
      }

      else if (count > 1) {
        insert_assoc(bool_enc_owned_layers, (node_ptr) lyr,
                     PTR_FROM_INT(node_ptr, count-1));
      }

      iter = ListIter_get_next(iter);
    }
  }

  /* base class deinitialization */
  base_enc_deinit(BASE_ENC(self));

  /* static members deinitialization: */
  bool_enc_instances -= 1;
  if (bool_enc_instances == 0) {
    /* this is the last instance */
    nusmv_assert(boolean_type != (node_ptr) NULL);
    free_node(boolean_type);
    boolean_type = (node_ptr) NULL;

    nusmv_assert(bool_enc_owned_layers != (hash_ptr) NULL);
    free_assoc(bool_enc_owned_layers);
    bool_enc_owned_layers = (hash_ptr) NULL;
  }
}



/**Function********************************************************************

  Synopsis           [Encodes all variables within the given layer]

  Description [A new layer will be constructed if there is not yet
  any.  The new layer will be called ${layer_name}_bool and will be
  added to the symbol table that self uses. The new layer will be
  locked by self either until the layer is was originally created from
  is released or until self is destroyed. Given a committed layer, it
  is always possible to obtain the corresponding created boolean layer
  by calling BoolEnc_scalar_layer_to_bool_layer. ]

  SideEffects        [A new layer will be created if not already existing]

  SeeAlso            [bool_enc_remove_layer]

******************************************************************************/
VIRTUAL void bool_enc_commit_layer(BaseEnc_ptr base_enc,
                                   const char* layer_name)
{
  BoolEnc_ptr self;
  SymbLayer_ptr src_layer, dest_layer;
  const char* dest_layer_name;
  SymbLayerIter iter;

  self = BOOL_ENC(base_enc);

  /* Calls the base method to add this layer */
  base_enc_commit_layer(base_enc, layer_name);

  src_layer = SymbTable_get_layer(BASE_ENC(self)->symb_table, layer_name);

  /* queries for the corresponding boolean layer, to see if it is
     already up and running: */
  dest_layer_name = BoolEnc_scalar_layer_to_bool_layer(layer_name);
  dest_layer = SymbTable_get_layer(BASE_ENC(self)->symb_table, dest_layer_name);

  if (dest_layer == SYMB_LAYER(NULL)) {
    /* does not exist yet, creates one. It will be created with the same policy
       the layer it derives from has */
    dest_layer = SymbTable_create_layer(BASE_ENC(self)->symb_table,
                                        dest_layer_name,
                                        SymbLayer_get_insert_policy(src_layer));

    /* encoders become the new layer owners */
    nusmv_assert(find_assoc(bool_enc_owned_layers, (node_ptr) dest_layer) ==
                 (node_ptr) NULL);
    insert_assoc(bool_enc_owned_layers, (node_ptr) dest_layer,
                 PTR_FROM_INT(node_ptr, 1));
  }
  else { /* if it is a layer whose owner is a another bool enc,
            increments the counter of users */
    int count = PTR_TO_INT(
                 find_assoc(bool_enc_owned_layers, (node_ptr) dest_layer));
    nusmv_assert(count >= 0);
    if (count > 0) {
      insert_assoc(bool_enc_owned_layers, (node_ptr) dest_layer,
                   PTR_FROM_INT(node_ptr, count+1));
    }
  }

  /* becomes an user of the bool layer: */
  base_enc_commit_layer(base_enc, dest_layer_name);

  /* now encodes all variables within the given layer, and puts them into the
     boolean layer */
  SYMB_LAYER_FOREACH(src_layer, iter, STT_VAR) {
    node_ptr var = SymbLayer_iter_get_symbol(src_layer, &iter);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr, "BoolEnc: encoding variable '");
      print_node(nusmv_stderr, var);
      fprintf(nusmv_stderr, "'...\n");
    }

    bool_enc_encode_var(self, var, src_layer, dest_layer);
  }
}


/**Function********************************************************************

  Synopsis           [Removes the encoding of all variables occurring within
  the given layer, and those that had been created within the corresponding
  boolean layer during the boolean encoding. Then releases both the layers,
  and removes the boolean layer from the symbol table.]

  Description        [  WARNING: If the layer has been
  renamed after having been committed, it is the *new* name (the name
  the layer has when it is being removed) that must be used, and *not*
  the name that had been used when commiting it.]

  SideEffects        []

  SeeAlso            [bool_enc_commit_layer]

******************************************************************************/
VIRTUAL void bool_enc_remove_layer(BaseEnc_ptr base_enc,
                                   const char* layer_name)
{
  BoolEnc_ptr self;
  SymbLayer_ptr layer, bool_layer;
  SymbLayerIter iter;
  const char* bool_layer_name;
  SymbType_ptr type;

  self = BOOL_ENC(base_enc);

  bool_layer_name = BoolEnc_scalar_layer_to_bool_layer(layer_name);
  layer = SymbTable_get_layer(BASE_ENC(self)->symb_table, layer_name);
  bool_layer = SymbTable_get_layer(BASE_ENC(self)->symb_table, bool_layer_name);

  /* removes all encodings from vars within layer */
  SYMB_LAYER_FOREACH(layer, iter, STT_VAR) {
    node_ptr var = SymbLayer_iter_get_symbol(layer, &iter);

    type = SymbTable_get_var_type(BASE_ENC(self)->symb_table, var);

    /*  Only ENUM and WORD types are implemented at the moment */
    switch (SymbType_get_tag(type)) {
    case SYMB_TYPE_BOOLEAN:
    case SYMB_TYPE_ENUM: /* ENUM type */
    case SYMB_TYPE_UNSIGNED_WORD: /* Word type */
    case SYMB_TYPE_SIGNED_WORD: /* Word type */
      if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
        fprintf(nusmv_stderr, "BoolEnc: removing encoding of variable '");
        print_node(nusmv_stderr, var);
        fprintf(nusmv_stderr, "'...\n");
      }

      if (bool_enc_get_var_encoding(self, var) != Nil) {
        NodeList_ptr bits;
        ListIter_ptr iter;

        /* Gets rid of all bits that were created by the encoding process
           of this variable */
        bits = BoolEnc_get_var_bits(self, var);
        iter = NodeList_get_first_iter(bits);
        while (!ListIter_is_end(iter)) {
          node_ptr bit;
          bit = NodeList_get_elem_at(bits, iter);
          if (bool_enc_get_var_encoding(self, bit) != Nil) {
            bool_enc_set_var_encoding(self, bit, Nil);
          }
          iter = ListIter_get_next(iter);
        }
        NodeList_destroy(bits);

        /* gets rid of the var's encoding and mask as well */
        bool_enc_set_var_encoding(self, var, Nil);
        if (SymbType_is_enum(type) && !SymbType_is_boolean(type)) {
          insert_assoc(self->var2mask, var, Nil);
        }
      }
      break;
    case SYMB_TYPE_WORDARRAY: /* WordArray type */
      fprintf(nusmv_stderr, "Unable to booleanize WordArrays.\n");
      nusmv_assert((false));
      break;
    case SYMB_TYPE_INTEGER:
    case SYMB_TYPE_REAL:
      /* for cegar hybrid added these two cases to ignore encoding
         for reals and integers */
      break;

      /* For s3ms added this case */
    case SYMB_TYPE_STRING: /* for testing purposes behaviour is
                              fake here */
      break;
    default:
      error_unreachable_code();
    }
  } /* end of loops over vars */

  /* cleans up the booleanizer cache as well */
  //Compile_cleanup_booleanizer_cache_about(BASE_ENC(self)->symb_table, vars);

  /* Calls the base method to get rid of both the source and boolean layer */
  base_enc_remove_layer(base_enc, layer_name);
  base_enc_remove_layer(base_enc, bool_layer_name);

  /* Removes the boolean layer from the symb table if this
     is the last user of it */
  {
    int count = PTR_TO_INT(
          find_assoc(bool_enc_owned_layers, (node_ptr) bool_layer));
    nusmv_assert(count >= 0);

    if (count != 0) {
      insert_assoc(bool_enc_owned_layers, (node_ptr) bool_layer,
                   PTR_FROM_INT(node_ptr, count-1));

      if (count == 1) {
        SymbTable_remove_layer(BASE_ENC(self)->symb_table, bool_layer);
      }
    }
  }
}



/**Function********************************************************************

  Synopsis           [Given the name of a scalar layer, a name of the
  corresponding boolean layer is returned.]

  Description        [Returned string should NOT be modified or freed.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
const char* BoolEnc_scalar_layer_to_bool_layer(const char* layer_name)
{
  char* bool_layer_name;
  string_ptr str;

  bool_layer_name = ALLOC(char, strlen(layer_name) +
                          strlen(BOOL_ENC_DEFAULT_LAYER_SUFFIX) + 1);
  nusmv_assert(bool_layer_name != (char*) NULL);
  strcpy(bool_layer_name, layer_name);
  strcat(bool_layer_name, BOOL_ENC_DEFAULT_LAYER_SUFFIX);

  /* the strings are used to avoid caring about memory */
  str = find_string(bool_layer_name);
  FREE(bool_layer_name);

  return str_get_text(str);
}

/**Function********************************************************************

  Synopsis    [Determines if a layer name corresponds to a bool layer]

  Description [Given the name of a layer, returns true if it is the
  name of a boolean layer.]

  SideEffects  [None]

  SeeAlso      [BoolEnc_scalar_layer_to_bool_layer]

******************************************************************************/
boolean BoolEnc_is_bool_layer(const char* layer_name)
{
  unsigned name_length;
  unsigned bool_length = strlen(BOOL_ENC_DEFAULT_LAYER_SUFFIX);

  nusmv_assert(NIL(char) != layer_name);
  name_length = strlen(layer_name);

  return (name_length > bool_length) && \
    (0 == strcmp(layer_name + name_length - bool_length,
                 BOOL_ENC_DEFAULT_LAYER_SUFFIX));
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The BoolEnc class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static VIRTUAL void bool_enc_finalize(Object_ptr object, void* dummy)
{
  BoolEnc_ptr self = BOOL_ENC(object);

  bool_enc_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Encodes a single variable ]

  Description [If it is a scalar variable, its values are expanded and a
  set of bits (new boolean variables) will be created within the
  dest_layer. All leaves (constant values of the values) will be
  created within the src_layer, it they are not defined yet.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void
bool_enc_encode_var(BoolEnc_ptr self, node_ptr var,
                    SymbLayer_ptr src_layer, SymbLayer_ptr dest_layer)
{
  node_ptr scalar_enc;
  SymbType_ptr type;

  type = SymbTable_get_var_type(BASE_ENC(self)->symb_table, var);

  /*  Only ENUM and WORD types are implemented at the moment */
  switch (SymbType_get_tag(type)) {
  case SYMB_TYPE_BOOLEAN:
    scalar_enc = boolean_type;
    break;
  case SYMB_TYPE_ENUM: { /* ENUM type */
    node_ptr values = SymbType_get_enum_type_values(type);
    nusmv_assert(Nil != values);
    scalar_enc = bool_enc_encode_scalar_var(self, var, 0, values,
                                            src_layer, dest_layer);
    break;
  }

  case SYMB_TYPE_UNSIGNED_WORD: /* unsigned and signed Word types */
  case SYMB_TYPE_SIGNED_WORD: {
    /* encode Word as an array of bits (ADD trees) */
    int width = SymbType_get_word_width(type);
    int suffix;
    node_ptr iter;
    node_ptr bits = Nil;
    /* higher bits are submitted first to make them higher in the BDD var order */
    for (suffix = width-1; suffix >= 0; --suffix) {
      node_ptr bitVar = BoolEnc_make_var_bit(self, var, suffix);
      /* declare a new boolean var -- bit of the Word */
      if (! SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, bitVar)) {
        /* the type is created every time, because the "reset" frees them */
        SymbType_ptr type = SymbType_create(SYMB_TYPE_BOOLEAN, Nil);

        if (SymbTable_is_symbol_input_var(BASE_ENC(self)->symb_table, var)) {
          SymbLayer_declare_input_var(dest_layer, bitVar, type);
        }
        else if (SymbTable_is_symbol_state_var(BASE_ENC(self)->symb_table, var)) {
          SymbLayer_declare_state_var(dest_layer, bitVar, type);
        }
        else {
          SymbLayer_declare_frozen_var(dest_layer, bitVar, type);
        }

        bool_enc_set_var_encoding(self, bitVar, boolean_type);
      }
      /* create a list of bits (high bits now goes last) */
      bits = cons(bitVar, bits);
    } /* for */
    /* reverese the list (high bit will go first) and find_node it.
       NB: representation of a variable is given by node_ptr and
       (not array_t, for example), and find_node is required because
       the destructor does not free any memory
    */
    for (scalar_enc = Nil, iter = bits; iter != Nil; iter = cdr(iter)) {
      scalar_enc = find_node(CONS, car(iter), scalar_enc);
    }
    free_list(bits);
    /* wrap the list into a unsigned WORD node =>
     result is always *unsigned* because it is just representation
     and the type is of no importance */
    scalar_enc = find_node(UNSIGNED_WORD,
                           scalar_enc,
                           find_node(NUMBER, NODE_FROM_INT(width), Nil));
    break;
  }

  case SYMB_TYPE_WORDARRAY: /* WordArray type */ {
    fprintf(nusmv_stderr, "Unable to booleanize WordArrays.\n");
    nusmv_assert((false));
  }

    /* skip array types */
  case SYMB_TYPE_ARRAY:
    return;

    /* for cegar hybrid added these two cases to ignore encoding
       for reals and integers */
  case SYMB_TYPE_INTEGER:
  case SYMB_TYPE_REAL:
    return;

    /* Added case for s3ms */
  case SYMB_TYPE_STRING: /* for testing purposes behaviour is fake here */
    return;

  default: error_unreachable_code(); /* no other kinds of types are implemented */
  } /* switch */

  bool_enc_set_var_encoding(self, var, scalar_enc);
}



/**Function********************************************************************

  Synopsis [Encodes a scalar variable, by creating all boolean vars
  (bits) needed to encode the var itself. Created bool vars are pushed
  within the given destination layer. ]

  Description [The returned structure is a tree, whose internal nodes
  are ITE nodes or BITS variables, and leaves are constants values.

  <expr> :: ITE ( COLON (bit, left), right) |
            constant

  where bit is a variable name (a bit), and left and right are <expr>.
  ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr
bool_enc_encode_scalar_var(BoolEnc_ptr self, node_ptr name, int suffix,
                           node_ptr values,
                           SymbLayer_ptr src_layer, SymbLayer_ptr dest_layer)
{
  node_ptr result;

  { /* declare constants if needed */
    node_ptr iter;
    for (iter=values; iter != Nil; iter = cdr(iter)) {
      node_ptr val = car(iter);
      if (SymbLayer_can_declare_constant(src_layer, val)) {
        SymbLayer_declare_constant(src_layer, val);
      }
    }
  }

  { /* calculates the encoding, and declares needed bits */
    Set_t bits = Set_MakeEmpty();
    Set_Iterator_t iter;

    result = bool_enc_compute_set_encoding(self, values, name, suffix, &bits, true);

    SET_FOREACH(bits, iter) {
      node_ptr bit = (node_ptr) Set_GetMember(bits, iter);

      if (! SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, bit)) {
        SymbType_ptr type = SymbType_create(SYMB_TYPE_BOOLEAN, Nil);
        if (SymbTable_is_symbol_input_var(BASE_ENC(self)->symb_table, name)) {
          SymbLayer_declare_input_var(dest_layer, bit, type);
        }
        else if (SymbTable_is_symbol_state_var(BASE_ENC(self)->symb_table, name)) {
          SymbLayer_declare_state_var(dest_layer, bit, type);
        }
        else {
          SymbLayer_declare_frozen_var(dest_layer, bit, type);
        }
      }

      bool_enc_set_var_encoding(self, bit, boolean_type);
    }

    Set_ReleaseSet(bits);
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Return true if the given list is {TRUE,FALSE}]

  Description        [Return true if the given list is {TRUE,FALSE}]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean bool_enc_is_boolean_range(node_ptr values)
{
  /* Small optimization */
  if (values == boolean_range) return true;

  while (Nil != values) {
    node_ptr v = car(values);

    if (!(TRUEEXP == node_get_type(v) ||
          FALSEEXP == node_get_type(v))) {
      return false;
    }

    values = cdr(values);
  }

  return true;
}

/**Function********************************************************************

  Synopsis [Computes the boolean encoding of which can be used to
            represent a set of values.]

            Description [Constructs a ITEs tree whose leaves are
            the values occurring in the input set, in a logarithmic
            boolean encoding. Conditions of the ITEs are BIT nodes
            whose name is constructed by using bit_prefix and
            bit_suffix values.

            Returns the containing the boolean logarithmic
            encoding, and as output parameter the set of BITs nodes
            used in the encoding.]

  SideEffects        [BITs nodes are added to the out_bits set]

  SeeAlso            []

******************************************************************************/
#if BOOL_ENCODING_LOWER_TO_HIGHER_BALANCED

/* this is the default encoding */
static node_ptr
bool_enc_compute_set_encoding(const BoolEnc_ptr self, node_ptr set,
                              node_ptr bit_prefix, int bit_suffix,
                              Set_t* out_bits, boolean top)
{
  node_ptr var, left, right;

  /* Final case: we reached a leaf */
  if (cdr(set) == Nil) {
    return find_atom(car(set));
  }

  /* Intermediate case, declare the scalar variable */
  if ((true == top) && bool_enc_is_boolean_range(set)) {
    var = bit_prefix;
  }
  else {
    var = BoolEnc_make_var_bit(self, bit_prefix, bit_suffix);
  }
  *out_bits = Set_AddMember(*out_bits, (Set_Element_t) var);

  { /* Finally construct the sub binary tree, by decomposing left
       and right sides: */
    node_ptr ls_left;
    node_ptr ls_right;

    ls_left = even_elements(set);
    left  = bool_enc_compute_set_encoding(self, ls_left,
                                          bit_prefix, bit_suffix + 1,
                                          out_bits, false);
    free_list(ls_left);

    ls_right = odd_elements(set);
    right = bool_enc_compute_set_encoding(self, ls_right,
                                          bit_prefix, bit_suffix + 1,
                                          out_bits, false);
    free_list(ls_right);
  }

  return find_node(IFTHENELSE, find_node(COLON, var, left), right);
}

#elif BOOL_ENCODING_HIGHER_TO_LOWER_BALANCED
/* this is proved to increase performances */

static node_ptr
bool_enc_compute_set_encoding_aux(const BoolEnc_ptr self, node_ptr set,
                                  node_ptr bit_prefix, int bit_suffix,
                                  Set_t* out_bits, boolean top);

static node_ptr
bool_enc_compute_set_encoding(const BoolEnc_ptr self, node_ptr set,
                              node_ptr bit_prefix, int bit_suffix,
                              Set_t* out_bits, boolean top)
{
  /* 'max()' to fix issue 2563, as at least one bit is required */
  int bits = Utils_log2_round(max(llength(set)-1, 1));
  nusmv_assert(bits>0);
  return bool_enc_compute_set_encoding_aux(self, set, bit_prefix,
                                           bit_suffix + bits - 1,
                                           out_bits, top);
}

static node_ptr
bool_enc_compute_set_encoding_aux(const BoolEnc_ptr self, node_ptr set,
                                  node_ptr bit_prefix, int bit_suffix,
                                  Set_t* out_bits, boolean top)
{
  node_ptr var, left, right;

  /* Final case: we reached a leaf */
  if (cdr(set) == Nil) {
    nusmv_assert(bit_suffix <= 0);
    return find_atom(car(set));
  }

  /* Intermediate case, declare the scalar variable */
  if ((true == top) && bool_enc_is_boolean_range(set)) {
    var = bit_prefix;
  }
  else {
    var = BoolEnc_make_var_bit(self, bit_prefix, bit_suffix);
  }
  *out_bits = Set_AddMember(*out_bits, (Set_Element_t) var);

  { /* Finally construct the sub binary tree, by decomposing left
       and right sides: */
    node_ptr ls_left;
    node_ptr ls_right;

    ls_left = even_elements(set);
    left  = bool_enc_compute_set_encoding_aux(self, ls_left,
                                              bit_prefix, bit_suffix - 1,
                                              out_bits, false);
    free_list(ls_left);

    ls_right = odd_elements(set);
    right = bool_enc_compute_set_encoding_aux(self, ls_right,
                                              bit_prefix, bit_suffix - 1,
                                              out_bits, false);
    free_list(ls_right);
  }

  return find_node(IFTHENELSE, find_node(COLON, var, left), right);
}

#elif BOOL_ENCODING_HIGHER_TO_LOWER_INCREMENTAL
/* This still have higher bits at hiegher levels, and it is
   constructed more efficiency, but the encoding results
   unbalanced. However its incrementality may result in better
   performances. */

static node_ptr
bool_enc_compute_set_encoding(const BoolEnc_ptr self, node_ptr set,
                              node_ptr bit_prefix, int bit_suffix,
                              Set_t* out_bits, boolean top)
{
  array_t* array1;
  node_ptr res;
  node_ptr bit_list = Nil; /* used to reverse added bits in out_bits */

  array1 = array_alloc(node_ptr, 2);
  nusmv_assert((array_t*) NULL != array1);

  { /* first loop along the given list, to avoid one unneeded traversal */
    node_ptr var = Nil;
    node_ptr iter = set;
    while (iter != Nil) {
      /* are there at least two elements? */
      if (cdr(iter) != Nil) {
        node_ptr el;
        node_ptr el1 = car(iter);
        node_ptr el2 = cadr(iter);

        if (Nil == var) {
          if ((Expr_is_false(el1) && Expr_is_true(el2)) ||
              (Expr_is_false(el2) && Expr_is_true(el1))) {
            var = bit_prefix; /* reuse bit for boolean local encoding */
          }
          else {
            var = BoolEnc_make_var_bit(self, bit_prefix, bit_suffix);
          }
          bit_list = cons(var, bit_list);
        }
        el = find_node(IFTHENELSE, find_node(COLON, var, el1), el2);
        array_insert_last(node_ptr, array1, el);

        iter = cddr(iter); /* advance two elements */
      }
      else {
        /* append the last remaining element */
        array_insert_last(node_ptr, array1, car(iter));
        iter = cdr(iter); /* advance one element */
      }
    }
  }

  /* now proceed with the arrays only */
  while (array_n(array1) > 1) {
    array_t* array2;
    int idx;
    node_ptr var;

    array2 = array_alloc(node_ptr, array_n(array1) / 2 + 1);
    nusmv_assert((array_t*) NULL != array1);

    /* a new bit has to be created */
    bit_suffix += 1;
    var = Nil;

    idx = 0;
    /* traverses array1 greedily taking pairs into array2 */
    while (idx < array_n(array1)) {
      if (idx + 1 < array_n(array1)) {
        /* there are at least two elements */
        node_ptr el;
        node_ptr el1 = array_fetch(node_ptr, array1, idx);
        node_ptr el2 = array_fetch(node_ptr, array1, idx+1);

        if (Nil == var) {
          if ((Expr_is_false(el1) && Expr_is_true(el2)) ||
              (Expr_is_false(el2) && Expr_is_true(el1))) {
            var = bit_prefix; /* reuse bit for boolean local encoding */
          }
          else {
            var = BoolEnc_make_var_bit(self, bit_prefix, bit_suffix);
          }
          bit_list = cons(var, bit_list);
        }
        el = find_node(IFTHENELSE, find_node(COLON, var, el1), el2);

        array_insert_last(node_ptr, array2, el);
        idx += 2;
      }
      else {
        /* last element */
        array_insert_last(node_ptr, array2,
                          array_fetch(node_ptr, array1, idx));
        idx += 1;
      }
    }

    { /* now array2 substitutes array1 */
      array_t* tmp = array1;
      array1 = array2;
      array_free(tmp); /* frees previous array1 */
    }
  }

  { /* dumps all collected bits in reversed order, to keep the
       right order (higher indices before lower) */
    for (; Nil!=bit_list; bit_list=cdr(bit_list)) {
      *out_bits = Set_AddMember(*out_bits, (Set_Element_t) car(bit_list));
    }
  }

  /* clean up and exit */
  nusmv_assert(array_n(array1) == 1);
  res = array_fetch(node_ptr, array1, 0);
  array_free(array1);
  free_list(bit_list);
  return res;
}
#else
#error "Invalid bool encoding"
#endif


/**Function********************************************************************

  Synopsis           [Associates the given variable with the specified
  boolean encoding]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
bool_enc_set_var_encoding(BoolEnc_ptr self, node_ptr name, node_ptr enc)
{ insert_assoc(self->var2enc, name, enc); }


/**Function********************************************************************

  Synopsis           [Given a variable, returns its boolean encoding, or NULL
  if not encoded]

  Description        [Private service]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr
bool_enc_get_var_encoding(const BoolEnc_ptr self, node_ptr name)
{ return find_assoc(self->var2enc, name); }


/**Function********************************************************************

  Synopsis           [Fills the given list with the BIT vars which
  occurs into the given var encoding]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bool_enc_traverse_encoding(const BoolEnc_ptr self,
                                       node_ptr enc, NodeList_ptr list)
{
  node_ptr bit;

  /* constant or number terminate (numbers are not stored as constants): */
  if ( SymbTable_is_symbol_constant(BASE_ENC(self)->symb_table, enc)
       || (node_get_type(enc) == NUMBER) || (enc == boolean_type)) return;

  if (node_get_type(enc) == IFTHENELSE) { /* usual IFTHENELSE encoding */
    bit = caar(enc);
    if (! NodeList_belongs_to(list, bit)) NodeList_append(list, bit);

    bool_enc_traverse_encoding(self, cdar(enc), list); /* 'then' */
    bool_enc_traverse_encoding(self, cdr(enc), list);      /* 'else' */
  }
  else if (node_get_type(enc) == UNSIGNED_WORD) { /* Word, i.e. array of bit-vars */
    node_ptr iter;
    for (iter = car(enc); iter != Nil; iter = cdr(iter)) {
      if(!NodeList_belongs_to(list, car(iter))) NodeList_append(list, car(iter));
    }
  }
  /* no other kind of node can appear at this level: */
  else error_unreachable_code();
}


/**Function********************************************************************

  Synopsis           [Given a variable, it returns the mask of its encoding]

  Description        [Returns an expression representing the mask that
  removes repetitions of leaves in a variable encoding.  This function
  assumes that the order in which we encounter variables in the
  expression representing the boolean encoding is the same as cube.

  As an example of what this function does, let us consider a variable
  x having range 0..4. It can be encoded with 3 bits are needed to
  encode it: x0, x1, x2. The encodeding performed by NuSMV is

     ITE(x0, ITE(x1, 1, 3), ITE(x1, 2, ITE(x2, 4,  0))).

  Thus x=2 corresponds to assignment !x0&x1 where x2 is a dont'care.
  Similarly for x=1 and x=3 (for x=0 and x=4) there is a unique
  complete assignment to the x0, x1, x2 variables that represent the
  respective encoding). This function fixes a value for x2 in the
  assignments representing x=2, x=1 and x=3 respectively (it force x2
  to be false). Thus it builds the formula in this case:

     ITE(x0, ITE(x2, 0, 1), ITE(x1, 1, ITE(x2, 0,  1)))

  that removes the redundant assignments where needed. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define _IS_LEAF(self,enc) (SymbTable_is_symbol_constant(BASE_ENC(self)->symb_table, enc) || \
                            ((Nil != enc) && (node_get_type(enc) == NUMBER)) || \
                            (enc == boolean_type))
#define _COND(enc) car(car(enc))
#define _THEN(enc) cdr(car(enc))
#define _ELSE(enc) cdr(enc)
#define _IS_ITE(enc) ((enc != Nil) && (IFTHENELSE == node_get_type(enc)) && \
                      (Nil != car(enc)) && (COLON ==  node_get_type(car(enc))))

static node_ptr bool_enc_get_var_mask_recur(const BoolEnc_ptr self,
                                            node_ptr enc,
                                            NodeList_ptr cube,
                                            ListIter_ptr cube_iter)
{
  node_ptr res;
  SymbTable_ptr symb_table = BaseEnc_get_symb_table(BASE_ENC(self));

  if (ListIter_is_end(cube_iter)) {
    /* We reached the end of the cube:
       we must be guaranteed to be on a leaf of the DAG */
    nusmv_assert(_IS_LEAF(self, enc));
    res = Expr_true();
  }
  else {
    node_ptr var = NodeList_get_elem_at(cube, cube_iter);

    if (_IS_LEAF(self, enc) || _COND(enc) != var) {
      /* There is a gap:
         we assign a value to missing variables in cube */
      node_ptr t = bool_enc_get_var_mask_recur(self,
                                               enc,
                                               cube,
                                               ListIter_get_next(cube_iter));

      res = Expr_ite(var, Expr_false(), t, symb_table);
    }
    else {
      /* It is a variable:
         we keep visiting the dag, searching for gaps to fill */

      /* Assumption that the order in which we encounter variables in
         the enc is the same as cube */
      nusmv_assert(_COND(enc) == var);

      node_ptr t = bool_enc_get_var_mask_recur(self, _THEN(enc), cube,
                                               ListIter_get_next(cube_iter));

      node_ptr e = bool_enc_get_var_mask_recur(self, _ELSE(enc), cube,
                                               ListIter_get_next(cube_iter));

      res = Expr_ite(_COND(enc), t, e, symb_table);
    }
  }

  return res;
}

/**AutomaticEnd***************************************************************/
