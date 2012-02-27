/**CFile***********************************************************************

   FileName    [NormalizerPsl.c]

   PackageName [node.normalizers]

   Synopsis    [Implementaion of class 'NormalizerPsl']

   Description []

   SeeAlso     [NormalizerPsl.h]

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

#include "NormalizerPsl.h"
#include "NormalizerPsl_private.h"

#include "parser/psl/psl_symbols.h"

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
/* See 'NormalizerPsl_private.h' for class 'NormalizerPsl' definition. */

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

static void normalizer_psl_finalize ARGS((Object_ptr object, void* dummy));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The NormalizerPsl class constructor]

   Description        [The NormalizerPsl class constructor]

   SideEffects        []

   SeeAlso            [NormalizerPsl_destroy]

******************************************************************************/
NormalizerPsl_ptr NormalizerPsl_create(const char* name)
{
  NormalizerPsl_ptr self = ALLOC(NormalizerPsl, 1);
  NORMALIZER_PSL_CHECK_INSTANCE(self);

  normalizer_psl_init(self, name,
                      NUSMV_PSL_SYMBOL_FIRST,
                      NUSMV_PSL_SYMBOL_LAST - NUSMV_PSL_SYMBOL_FIRST);
  return self;
}




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The NormalizerPsl class private initializer]

   Description        [The NormalizerPsl class private initializer]

   SideEffects        []

   SeeAlso            [NormalizerPsl_create]

******************************************************************************/
void normalizer_psl_init(NormalizerPsl_ptr self,
                         const char* name, int low, size_t num)
{
  /* base class initialization */
  normalizer_base_init(NORMALIZER_BASE(self), name, low,
                       num, true /*handles NULL*/);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = normalizer_psl_finalize;
  OVERRIDE(NormalizerBase, normalize_node) = normalizer_psl_normalize_node;

}


/**Function********************************************************************

   Synopsis           [The NormalizerPsl class private deinitializer]

   Description        [The NormalizerPsl class private deinitializer]

   SideEffects        []

   SeeAlso            [NormalizerPsl_destroy]

******************************************************************************/
void normalizer_psl_deinit(NormalizerPsl_ptr self)
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
node_ptr normalizer_psl_normalize_node(NormalizerBase_ptr self, node_ptr node)
{
  if (Nil == node) return Nil;

  return find_node(node_get_type(node),
                   _THROW(car(node)),
                   _THROW(cdr(node)));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions*/
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis[The NormalizerPsl class virtual finalizer]

   Description [Called by the class destructor]

   SideEffects []

   SeeAlso []

******************************************************************************/
static void normalizer_psl_finalize(Object_ptr object, void* dummy)
{
  NormalizerPsl_ptr self = NORMALIZER_PSL(object);

  normalizer_psl_deinit(self);
  FREE(self);
}

/**AutomaticEnd***************************************************************/

