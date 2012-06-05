/**CHeaderFile*****************************************************************

  FileName    [BaseEvaluator.h]

  PackageName [eval]

  Synopsis    [Public interface of class 'BaseEvaluator']

  Description []

  SeeAlso     [BaseEvaluator.c]

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``eval'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

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

  Revision    [$Id: $]

******************************************************************************/
#ifndef __BASE_EVALUATOR_H__
#define __BASE_EVALUATOR_H__

#include "utils/utils.h"
#include "utils/assoc.h"
#include "utils/object.h"

#include "fsm/sexp/Expr.h"
#include "compile/symb_table/SymbTable.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class BaseEvaluator]

  Description []

******************************************************************************/
typedef struct BaseEvaluator_TAG*  BaseEvaluator_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class BaseEvaluator]

  Description [These macros must be used respectively to cast and to check
  instances of class BaseEvaluator]

******************************************************************************/
#define BASE_EVALUATOR(self) \
         ((BaseEvaluator_ptr) self)

#define BASE_EVALUATOR_CHECK_INSTANCE(self) \
         (nusmv_assert(BASE_EVALUATOR(self) != BASE_EVALUATOR(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN BaseEvaluator_ptr BaseEvaluator_create ARGS((void));

EXTERN void BaseEvaluator_destroy ARGS((BaseEvaluator_ptr self));

EXTERN void BaseEvaluator_set_context ARGS((BaseEvaluator_ptr self,
                                            const SymbTable_ptr st,
                                            const hash_ptr env));

EXTERN Expr_ptr BaseEvaluator_evaluate ARGS((BaseEvaluator_ptr self,
                                             Expr_ptr const_expr));

/**AutomaticEnd***************************************************************/



#endif /* __BASE_EVALUATOR_H__ */
