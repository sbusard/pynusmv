/**CHeaderFile*****************************************************************

  FileName    [PredicateNormaliser.h]

  PackageName [compile]

  Synopsis    [Public interface for a predicate-normaliser class]

  Description [
  The purpose of a predicate normaliser is to take a symbolic
  expression (node_ptr), normalise predicates and return a newly
  created expressions with normlised predicates.  A
  predicate-normalised expression is an expression where none of
  not-boolean subexpressions may have a boolean subexpression, i.e.
  only boolean expressions may have boolean subexpressions.
  Normalisation is done by (creating and) pushing IfThenElse
  expression up to the root of not-boolean subexpression.  For
  example,

     "case a : 3; 1 : 4; esac + 2 = 7" 

  have boolean expression "a" as a subexpression of not-boolean
  expression "case ...".  The normalised version will look like 
     "case a : 3 + 2 = 7; 1 : 4 + 2 = 7; esac" 


  This is a stand-alone class. This class needs only a type checker
  -- to get the type of input expression and type check the generated
  (returned) expressions.
  ]
  
  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2. 
  Copyright (C) 2005 by FBK-irst. 

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

  Revision    [$Id: PredicateNormaliser.h,v 1.1.2.1.6.4 2009-09-17 11:49:47 nusmv Exp $]
******************************************************************************/


#ifndef __PREDICATE_NORMALISER_H__
#define __PREDICATE_NORMALISER_H__

#include "compile/symb_table/SymbTable.h"
#include "set/set.h"
#include "utils/utils.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct PredicateNormaliser_TAG* PredicateNormaliser_ptr;

#define PREDICATE_NORMALISER(x) \
         ((PredicateNormaliser_ptr) x)

#define PREDICATE_NORMALISER_CHECK_INSTANCE(x) \
         ( nusmv_assert(PREDICATE_NORMALISER(x) != PREDICATE_NORMALISER(NULL)) )


/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */

EXTERN PredicateNormaliser_ptr 
PredicateNormaliser_create ARGS((SymbTable_ptr st));

EXTERN void  PredicateNormaliser_destroy ARGS((PredicateNormaliser_ptr self));

EXTERN node_ptr 
PredicateNormaliser_normalise_expr ARGS((PredicateNormaliser_ptr self, 
                                         node_ptr expr));

EXTERN node_ptr 
PredicateNormaliser_normalise_specification ARGS((PredicateNormaliser_ptr self,
                                                  node_ptr expr));


EXTERN void 
PredicateNormaliser_get_predicates_only 
ARGS((const PredicateNormaliser_ptr self,
      Set_t* preds, node_ptr expr));

EXTERN void 
PredicateNormaliser_print_predicates_only 
ARGS((const PredicateNormaliser_ptr self,
      FILE* stream,
      node_ptr expr));

#endif /* __PREDICATE_NORMALISER_H__ */
