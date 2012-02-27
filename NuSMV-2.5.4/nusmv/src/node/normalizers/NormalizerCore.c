/**CFile***********************************************************************

   FileName    [NormalizerCore.c]

   PackageName [node.normalizers]

   Synopsis    [Implementaion of class 'NormalizerCore']

   Description []

   SeeAlso     [NormalizerCore.h]

   Author      [Alessandro Mariotti]

   Copyright   [
   This file is part of the ``node.normalizers'' package of NuSMV version 2.
   Copyright (C) 2006 by FBK-irst.

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

#include "NormalizerCore.h"
#include "NormalizerCore_private.h"

#include "parser/symbols.h"

#include "utils/WordNumber.h"
#include "utils/utils.h"
#include "utils/ustring.h"
#include "utils/error.h"
#include "compile/compile.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'NormalizerCore_private.h' for class 'NormalizerCore' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

   Synopsis           [Short way of calling normalizer_base_throw_normalize_node]

   Description        [Use this macro to recursively recall normalize_node]

   SeeAlso            []

******************************************************************************/
#define _THROW(n)                                                       \
  normalizer_base_throw_normalize_node(NORMALIZER_BASE(self), n)

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void normalizer_core_finalize ARGS((Object_ptr object, void* dummy));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The NormalizerCore class constructor]

   Description        [The NormalizerCore class constructor]

   SideEffects        []

   SeeAlso            [NormalizerCore_destroy]

******************************************************************************/
NormalizerCore_ptr NormalizerCore_create(const char* name)
{
  NormalizerCore_ptr self = ALLOC(NormalizerCore, 1);
  NORMALIZER_CORE_CHECK_INSTANCE(self);

  normalizer_core_init(self, name,
                       NUSMV_CORE_SYMBOL_FIRST,
                       NUSMV_CORE_SYMBOL_LAST - NUSMV_CORE_SYMBOL_FIRST);
  return self;
}




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The NormalizerCore class private initializer]

   Description        [The NormalizerCore class private initializer]

   SideEffects        []

   SeeAlso            [NormalizerCore_create]

******************************************************************************/
void normalizer_core_init(NormalizerCore_ptr self,
                          const char* name, int low, size_t num)
{
  /* base class initialization */
  normalizer_base_init(NORMALIZER_BASE(self), name, low,
                       num, true /*handles NULL*/);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = normalizer_core_finalize;
  OVERRIDE(NormalizerBase, normalize_node) = normalizer_core_normalize_node;

}


/**Function********************************************************************

   Synopsis           [The NormalizerCore class private deinitializer]

   Description        [The NormalizerCore class private deinitializer]

   SideEffects        []

   SeeAlso            [NormalizerCore_destroy]

******************************************************************************/
void normalizer_core_deinit(NormalizerCore_ptr self)
{
  /* members deinitialization */


  /* base class initialization */
  normalizer_base_deinit(NORMALIZER_BASE(self));
}


/**Function********************************************************************

   Synopsis    [Virtual menthod that normalizes the given node
   (core nodes are handled here)]

   Description []

   SideEffects []

   SeeAlso     []

******************************************************************************/
node_ptr normalizer_core_normalize_node(NormalizerBase_ptr self, node_ptr node)
{
  if (Nil == node) return Nil;

  switch (node_get_type(node)) {
  case FAILURE:
  case TRUEEXP:
  case FALSEEXP:
  case BOOLEAN:
  case NUMBER:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case ATOM:
  case BIT:
    return find_atom(node);

  case NUMBER_SIGNED_WORD:
  case NUMBER_UNSIGNED_WORD:
    {
      const WordNumber_ptr num = (const WordNumber_ptr) car(node);
      return find_node(node_get_type(node),
                       NODE_PTR(WordNumber_normalize(num)), Nil);
    }
  default:
    break;
  }

  return find_node(node_get_type(node), _THROW(car(node)), _THROW(cdr(node)));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions*/
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis[The NormalizerCore class virtual finalizer]

   Description [Called by the class destructor]

   SideEffects []

   SeeAlso []

******************************************************************************/
static void normalizer_core_finalize(Object_ptr object, void* dummy)
{
  NormalizerCore_ptr self = NORMALIZER_CORE(object);

  normalizer_core_deinit(self);
  FREE(self);
}

/**AutomaticEnd***************************************************************/

